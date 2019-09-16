// Copyright (c) 2018, Boris Bogaerts
// All rights reserved.

// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:

// 1. Redistributions of source code must retain the above copyright 
// notice, this list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright 
// notice, this list of conditions and the following disclaimer in the 
// documentation and/or other materials provided with the distribution.

// 3. Neither the name of the copyright holder nor the names of its 
// contributors may be used to endorse or promote products derived from 
// this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "vr_renderwindow_support.h"
#include <vector>
#include "extApi.h"
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>

#include <vtkOpenVRInteractorStyle.h>
#include <vtkOpenVRModel.h>

#include <vtkOpenVRMenuWidget.h>
#include <vtkOpenVRMenuRepresentation.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkOutputWindow.h>
#include <vtkFileOutputWindow.h>
#include <vtkOBJImporter.h>
#include <vtkActorCollection.h>
#include <vtkPolyDataMapper.h>

#include "vrep_plot_container.h"
// Sorry this is purely vtk's fault, stupid function handles etc
class miniClass {
public:
	std::string name;
	int clientID;
};

class eventCatcher : public vtkOpenVRInteractorStyle
{
public:
	eventCatcher() {
		//SetDollyMotionFactor(10.0);
	}
	void OnButton3D(vtkEventData *edata) {
		int mode;
		bool trackpad = false;
		std::string signalName;
		vtkEventDataDeviceInput button = edata->GetAsEventDataDevice3D()->GetInput();
		vtkEventDataDevice controller = edata->GetAsEventDataDevice3D()->GetDevice();
		vtkEventDataAction action = edata->GetAsEventDataDevice3D()->GetAction();
		bool left = false;
		if (controller == vtkEventDataDevice::LeftController) {
			signalName = "L_";
			left = true;
		}
		else if (controller == vtkEventDataDevice::RightController) {
			signalName = "R_";
			left = false;
		}

		if (button == vtkEventDataDeviceInput::Trigger) {
			signalName.append("Trigger_");
		}else if (button == vtkEventDataDeviceInput::Grip) {
			signalName.append("Grip_");
		}else if (button == vtkEventDataDeviceInput::TrackPad) {
			trackpad = true;
			signalName.append("TrackPad_");
		}else if (button == vtkEventDataDeviceInput::Joystick) {
			trackpad = true;
			signalName.append("Joystick_");
			vtkOpenVRRenderWindowInteractor* rw = vtkOpenVRRenderWindowInteractor::SafeDownCast(this->Interactor); // get renderwindowinteractor
			float res[3];
			if (left) {
				rw->GetTouchPadPosition(vtkEventDataDevice::LeftController, vtkEventDataDeviceInput::Joystick, res);
				simxSetFloatSignal(clientID, "L_Joystick_pos_x", res[0], simx_opmode_oneshot); // get last touchpad position
				simxSetFloatSignal(clientID, "L_Joystick_pos_y", res[1], simx_opmode_oneshot);
				simxSetFloatSignal(clientID, "L_Joystick_pos_z", res[2], simx_opmode_oneshot); // whatever this is
			}
			else {
				rw->GetTouchPadPosition(vtkEventDataDevice::RightController, vtkEventDataDeviceInput::Joystick, res);
				simxSetFloatSignal(clientID, "R_Joystick_pos_x", res[0], simx_opmode_oneshot); // get last touchpad position
				simxSetFloatSignal(clientID, "R_Joystick_pos_y", res[1], simx_opmode_oneshot);
				simxSetFloatSignal(clientID, "R_Joystick_pos_z", res[2], simx_opmode_oneshot); // whatever this is
			}

		}else if (button == vtkEventDataDeviceInput::ApplicationMenu) {
			trackpad = true;
			signalName.append("ApplicationMenu_");
		}

		if (action == vtkEventDataAction::Press) {
			signalName.append("Press");
			mode = 1;
		}else if (action == vtkEventDataAction::Release) {
			signalName.append("Press");
			mode = 0;
		}else if (action == vtkEventDataAction::Touch) {
			signalName.append("Touch");
			mode = 1;
		}else if (action == vtkEventDataAction::Untouch) {
			signalName.append("Touch");
			mode = 0;
		}
		
		if ((controller == vtkEventDataDevice::LeftController) && (button == vtkEventDataDeviceInput::TrackPad)&& (action == vtkEventDataAction::Release)) {
			if ((grid != nullptr)){
				cout << endl << "Collormap toggle" << endl;
				grid->toggleMode();
			}
		}
		simxSetIntegerSignal(clientID, signalName.c_str(), mode, simx_opmode_oneshot);
		if (useVTKinteractor) {
			vtkOpenVRInteractorStyle::OnButton3D(edata);
		}
	}
	int clientID;
	vrep_volume_grid *grid = nullptr;
	bool useVTKinteractor = true;
protected:
};

// This one is a handle function for the vision sensor thread
void handleFunc(vr_renderwindow_support *sup) {
	cout << "Vision sensor thread activated" << endl;
	sup->visionSensorThread();
}

vr_renderwindow_support::vr_renderwindow_support(int cid, int ref, int interactor)
{ 
	refH = ref;
	setClientID(cid, interactor);
	chrono->coverage = &coverage; 
	chrono->scale = &scale; 
	// Now add spectator camera
	simxFloat *data;
	simxInt dataLength;
	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"checkForCam"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	if (data[0] != 0) {

	}

	vtkOutputWindow* ow = vtkOutputWindow::GetInstance();
	vtkFileOutputWindow* fow = vtkFileOutputWindow::New();
	fow->SetFileName("debug.log");
	if (ow)
	{
		ow->SetInstance(fow);
	}
	fow->Delete();
}

vr_renderwindow_support::~vr_renderwindow_support()
{

}

void vr_renderwindow_support::checkLayers() {
	simxInt val;
	simxGetIntegerSignal(clientID, (simxChar*)"VisibleLayers", &val, simx_opmode_streaming);
	for (int i = 0; i < vrepScene->getNumActors(); i++) {
		std::bitset<16> mybit = (std::bitset<16>)val;
		mybit = (mybit & visibilityLayer[i]) & visibilityLayer[i];
		vrepScene->getActor(i)->SetVisibility(mybit.any());
	}
}

void vr_renderwindow_support::updateRender() {
	vr_renderWindowInteractor->Render();
};

void vr_renderwindow_support::addVrepScene(vrep_scene_content *vrepSceneIn) {
	vrepScene = vrepSceneIn;

	vrepScene->vrep_get_object_pose();
	for (int i = 0; i < vrepScene->getNumActors(); i++) {
		renderer->AddActor(vrepScene->getActor(i));
		visibilityLayer.push_back((std::bitset<16>)vrepScene->getVisibilityLayer(i));
	}
	lastObject = vrepScene->getNumActors();
	for (int i = 0; i < vrepScene->getNumRenders(); i++) {
		renderer->AddActor(vrepScene->getPanelActor(i));
	}
	for (int i = 0; i < vrepScene->getNumberOfLights(); i++) {
		renderer->AddLight(vrepScene->getLight(i));
		//renderer->AddViewProp(vrepScene->getLightActor(i));
	}

	if (vrepScene->isVolumePresent()) {
		renderer->AddViewProp(vrepScene->getVolume());
		grid = vrepScene->vol;
		//renderer->ResetCamera();
	}
	else {
		grid = nullptr;
	}
}


void vr_renderwindow_support::dynamicAddObjects() {
	int result;
	simxGetIntegerSignal(clientID, "dynamic_load_request", &result, simx_opmode_streaming);
	if (result == 0) {
		return;
	}
	vrepScene->dynamicLoad();
	for (int i = lastObject; i < vrepScene->getNumActors(); i++) {
		vrepScene->getActor(i)->PickableOff();
		vrepScene->getActor(i)->GetProperty()->SetAmbient(0.7);
		vrepScene->getActor(i)->GetProperty()->SetDiffuse(0.5);
		renderer->AddActor(vrepScene->getActor(i));
	}
	lastObject = vrepScene->getNumActors();
	simxSetIntegerSignal(clientID, "dynamic_load_request", 0, simx_opmode_oneshot);
}

void vr_renderwindow_support::updatePose() {
	vrepScene->updateMainCamObjectPose();
}

void vr_renderwindow_support::syncData() {
	vrepScene->transferVisionSensorData();
}

void vr_renderwindow_support::visionSensorThread() {
	
	vrepScene->activateNewConnection(); // connect to vrep with a new port
	while (busy) {
		vrepScene->updateVisionSensorObjectPose();
		coverage = vrepScene->updateVisionSensorRender();
		dataReady = true;
		while (dataReady) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		};
	}
}

void vr_renderwindow_support::discoverDevices() {
	controller1_vrep = new vrep_controlled_object(clientID, refH);
	controller2_vrep = new vrep_controlled_object(clientID, refH);
	headset_vrep = new vrep_controlled_object(clientID, refH);
	
	headset_vrep->setName("Headset");
	headset_vrep->setDevice(vtkEventDataDevice::HeadMountedDisplay);

	controller1_vrep->setName("RightController");
	controller1_vrep->setDevice(vtkEventDataDevice::RightController);
		
	controller2_vrep->setName("LeftController");
	controller2_vrep->setDevice(vtkEventDataDevice::LeftController);
	//cout << "Number of devices : " << (int)vtkEventDataDevice::NumberOfDevices << endl;
	//vtkSmartPointer<vtkOpenVRModel> temp;
	//vr::IVRRenderModels* test = renderWindow->GetOpenVRRenderModels();
	////test->
	//for (int i = 1; i < 10; i++) {
	//	temp = renderWindow->GetTrackedDeviceModel((vr::TrackedDeviceIndex_t)i);
	//	if (temp != nullptr) {
	//		cout << << endl;
	//	}
	//}
}

void vr_renderwindow_support::synchronizeDevices() {
	headset_vrep->updatePosition(renderWindow, vr_renderWindowInteractor, vr_camera);
	controller1_vrep->updatePosition(renderWindow, vr_renderWindowInteractor, vr_camera);
	controller2_vrep->updatePosition(renderWindow, vr_renderWindowInteractor, vr_camera);

	// Send trackpad positions
	float res[3];
	vr_renderWindowInteractor->GetTouchPadPosition(vtkEventDataDevice::LeftController, vtkEventDataDeviceInput::TrackPad, res);
	simxSetFloatSignal(clientID, "L_Trackpad_pos_x", res[0], simx_opmode_oneshot); // get last touchpad position
	simxSetFloatSignal(clientID, "L_Trackpad_pos_y", res[1], simx_opmode_oneshot);
	simxSetFloatSignal(clientID, "L_Trackpad_pos_z", res[2], simx_opmode_oneshot); // whatever this is
	vr_renderWindowInteractor->GetTouchPadPosition(vtkEventDataDevice::RightController, vtkEventDataDeviceInput::TrackPad, res);
	simxSetFloatSignal(clientID, "R_Trackpad_pos_x", res[0], simx_opmode_oneshot); // get last touchpad position
	simxSetFloatSignal(clientID, "R_Trackpad_pos_y", res[1], simx_opmode_oneshot);
	simxSetFloatSignal(clientID, "R_Trackpad_pos_z", res[2], simx_opmode_oneshot); // whatever this is
}

void vr_renderwindow_support::updateText() {
	double pos[3];
	double wxyz[4];
	double ppos[3];
	double wdir[3];
	Tt->PostMultiply();
	Tt->Identity();
	Tt->RotateX(-90);
	Tt->Translate(-0.17*scale, 0 , 0.05*scale);
	txtActor->SetScale(0.01*scale);
	if (renderWindow->GetTrackedDeviceModel(vtkEventDataDevice::LeftController) == nullptr) { return; } // if no controller leave
	vr::TrackedDevicePose_t& vrPose = renderWindow->GetTrackedDevicePose(renderWindow->GetTrackedDeviceIndexForDevice(vtkEventDataDevice::LeftController)); // this was the problem
	vr_renderWindowInteractor->ConvertPoseToWorldCoordinates(vrPose, pos, wxyz, ppos, wdir);
	Tt->RotateWXYZ(wxyz[0], wxyz[1], wxyz[2], wxyz[3]);
	
	Tt->Translate(pos);
	Tt->Modified();
	if (textUpdateCounter < 30) {
		textUpdateCounter++;
		return;
	}
	char * tab2 = new char[chrono->getText().length() + 1];
	strcpy(tab2, chrono->getText().c_str());
	vecText->SetText(tab2);
	vecText->Modified();
	textUpdateCounter = 0;
}

// Callback function 
void activateMenuItem(vtkObject* caller, unsigned long eid, void* clientdata, void *calldata) {
	miniClass* test = reinterpret_cast<miniClass*>(clientdata);
	simxCallScriptFunction(test->clientID, (simxChar*)"Menu", sim_scripttype_childscript, (simxChar*)test->name.c_str()
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);
}

// Ugly code but VTK is behaving badly, this works
void fixMenu(std::vector<std::string> *text, std::vector<miniClass> *functionName, std::vector<vtkSmartPointer<vtkCallbackCommand>> *callback, eventCatcher *events, int clientID) {
	int menuH;
	simxInt *data;
	simxInt dataLength;
    simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"doWeHaveManu"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);

	bool even = false;
	events->GetMenu()->RemoveAllMenuItems();
	if (data[0]!=-1) {
		simxChar *stringData;
		simxCallScriptFunction(clientID, (simxChar*)"Menu", sim_scripttype_childscript, (simxChar*)"getMenuItems"
			, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL, NULL, &dataLength, &stringData, NULL, NULL, simx_opmode_blocking);
		int encounteredStrings = 0;
		int counter = 0;
		text->push_back(std::string());
		miniClass *temp;
		while (encounteredStrings<dataLength){
			if (stringData[counter] == char(0)) {
				encounteredStrings++; 
				even = !even;

				if (even) { // create new strings
					temp = new miniClass();
					temp->name = std::string();
					temp->clientID = clientID;
					functionName->push_back(*temp);
					vtkSmartPointer<vtkCallbackCommand> callbackTemp = vtkSmartPointer<vtkCallbackCommand>::New();
					callbackTemp->SetCallback(activateMenuItem);
					callback->push_back(callbackTemp);
				}
				else {
					text->push_back(std::string());
				}
			}
			else {
				if (even) {
					functionName->at(functionName->size()-1).name += ((char)stringData[counter]);
				}
				else {
					text->at(text->size()-1) += (char)stringData[counter];	
				}
			}
			counter++;
		}
	}
	for (int i = (dataLength / 2)-1; i >=0; i--) {
		std::string temp1 = functionName->at(i).name;
		std::string temp2 = text->at(i);
		callback->at(i)->SetClientData(&functionName->at(i));
		events->GetMenu()->PushFrontMenuItem(temp1.c_str(), temp2.c_str(), callback->at(i));
	}
}

void vr_renderwindow_support::activate_interactor() {
	vrepScene->vrep_get_object_pose();
	// Classical vtk pipeline to set up renderwindow etc with extra options
	renderer->SetActiveCamera(vr_camera);
	renderer->UseFXAAOn();
	//renderer->UseFXAAOff();
	renderer->SetBackground(1.0, 1.0, 1.0);

	// Do some aestetic thingies
	simxFloat *data;
	simxInt dataLength;
	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"getEstetics"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	if (dataLength > 0) {
		renderer->SetBackground(data[0], data[1], data[2]);
		renderer->SetBackground2(data[3], data[4], data[5]);
		renderer->SetGradientBackground(true);
	}

	
	renderer->SetAmbient(0.4, 0.4, 0.4);
	renderer->Modified();
	renderWindow->AddRenderer(renderer);
	renderWindow->SetMultiSamples(0);
	renderWindow->SetDesiredUpdateRate(90.0);
	
	renderer->LightFollowCameraOn();
	renderer->AutomaticLightCreationOff();
	int goodRender;
	simxGetIntegerSignal(clientID, "High_quality_render", &goodRender, simx_opmode_blocking); // whatever this is
	if (goodRender == 1) {
		renderer->UseShadowsOn();
	}
	renderer->TwoSidedLightingOff();
	//renderWindow->SetTrackHMD(true);
	renderWindow->PolygonSmoothingOn();
	renderWindow->Initialize();
	
	vr_renderWindowInteractor->SetRenderWindow(renderWindow);
	vr_renderWindowInteractor->Initialize();

	// Set up callback to capture interaction
	eventCatcher *events = new(eventCatcher); // define object that captures buttonpress
	events->clientID = clientID;
	events->useVTKinteractor = useInteractor;


	std::vector<std::string> text;
	std::vector<miniClass> functionName;
	std::vector<vtkSmartPointer<vtkCallbackCommand>> callback;
	fixMenu(&text, &functionName, &callback, events, clientID);
	
	if ((grid != nullptr) && (vrepScene->isVolumePresent())){
		cout << "Volume detected and connected" << endl;
		events->grid = grid;
	}
	vr_renderWindowInteractor->SetInteractorStyle(events); // set object which captures buttonpress
	cout << "Everithing loaded succesfully" << endl;
	
	// controllers
	discoverDevices();
	synchronizeDevices();

	// add text to
	vecText->SetText("vtkVectorText");
	extrude->SetInputConnection(vecText->GetOutputPort());
	extrude->SetExtrusionTypeToNormalExtrusion();
	extrude->SetVector(0, 0, 0.5);
	extrude->SetScaleFactor(1);
	txtMapper->SetInputConnection(extrude->GetOutputPort());
	txtActor->SetMapper(txtMapper);
	txtActor->SetScale(0.01);
	txtActor->GetProperty()->SetColor(0.53, 0.02, 0.02);
	txtActor->SetUserTransform(Tt);
	renderer->AddActor(txtActor);

	// Search for dynamic path object
	path = new pathObject(clientID);
	renderer->AddActor(path->getActor());
	renderer->Modified();
	
	// Manage vision sensor thread (if necessary)
	std::thread camThread;
	bool threadActivated = false;
	if (vrepScene->startVisionSensorThread()) {
		busy = true;
		camThread = std::thread(handleFunc, this);
		threadActivated = true;
	}else{
		camThread.~thread();
	}

	// Define plot container
	vrep_plot_container *pc = new vrep_plot_container(clientID, renderer);
	
	// Start the render loop
	while (true) {
		updatePose();
		synchronizeDevices();
		
		chrono->increment();
		scale = (float)vr_renderWindowInteractor->GetPhysicalScale();
	
		if (!useInteractor) {
			vr_renderWindowInteractor->SetPhysicalScale(1); // no scaling
		}
		vr_renderWindowInteractor->DoOneEvent(renderWindow, renderer); // actual vr render
		dynamicAddObjects(); // see if there are new objects in the scene, if so add them
		updateText(); // change framerate text
		path->update();
		checkLayers();

		// update plot container
		pc->update();

		if (isReady()) {
			syncData(); // transfer information of vision sensor in differend thread to this thread
			//grid->updatMap();
			chrono->increment2(); 
			vr_renderWindowInteractor->DoOneEvent(renderWindow, renderer); // render
			chrono->increment();
			setNotReady(); // resume seperate thread
		}
		
		if (simxGetLastCmdTime(clientID) <= 0) {
			busy = false;
			vr_renderWindowInteractor->TerminateApp(); // stop if the v-rep simulation is not running
			if (threadActivated) {
				camThread.join();
				camThread.~thread();
			}
			cout << endl << endl << endl;
			return;
		}
	}
}

