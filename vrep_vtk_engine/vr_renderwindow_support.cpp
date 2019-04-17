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

#include <thread>
#include <chrono>
//#include <vtkOutputWindow.h>
//#include <vtkFileOutputWindow.h>

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
		if (controller == vtkEventDataDevice::LeftController) {
			signalName = "L_";
		}
		else if (controller == vtkEventDataDevice::RightController) {
			signalName = "R_";
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

	/*vtkOutputWindow* ow = vtkOutputWindow::GetInstance();
	vtkFileOutputWindow* fow = vtkFileOutputWindow::New();
	fow->SetFileName("debug.log");
	if (ow)
	{
		ow->SetInstance(fow);
	}
	fow->Delete();*/
}

vr_renderwindow_support::~vr_renderwindow_support()
{

}

void vr_renderwindow_support::updateRender() {
	vr_renderWindowInteractor->Render();
};

void vr_renderwindow_support::addVrepScene(vrep_scene_content *vrepSceneIn) {
	vrepScene = vrepSceneIn;

	vrepScene->vrep_get_object_pose();
	for (int i = 0; i < vrepScene->getNumActors(); i++) {
		vrepScene->getActor(i)->PickableOff();
		vrepScene->getActor(i)->GetProperty()->SetAmbient(0.7);
		vrepScene->getActor(i)->GetProperty()->SetDiffuse(0.5);
		renderer->AddActor(vrepScene->getActor(i));
	}
	lastObject = vrepScene->getNumActors();
	for (int i = 0; i < vrepScene->getNumRenders(); i++) {
		renderer->AddActor(vrepScene->getPanelActor(i));
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
	//vrepScene->transferVisionSensorData(iren, win, ren);
	vrepScene->transferVisionSensorData();
}

void vr_renderwindow_support::visionSensorThread() {
	vrepScene->activateNewConnection(); // connect to vrep with a new port
	while (true) {
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

	controller1_vrep->setName("Controller1");
	controller1_vrep->setDevice(vtkEventDataDevice::RightController);
		
	controller2_vrep->setName("Controller2");
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

	renderer->AutomaticLightCreationOn();
	renderer->SetAutomaticLightCreation(true);
	renderer->LightFollowCameraOn();
	renderer->UseShadowsOff();
	renderer->Modified();

	renderWindow->AddRenderer(renderer);
	renderWindow->SetMultiSamples(0);
	renderWindow->SetDesiredUpdateRate(90.0);
	//renderWindow->SetTrackHMD(true);
	renderWindow->Initialize();

	vr_renderWindowInteractor->SetRenderWindow(renderWindow);
	vr_renderWindowInteractor->Initialize();

	// Set up callback to capture interaction
	eventCatcher *events = new(eventCatcher); // define object that captures buttonpress
	events->clientID = clientID;
	events->useVTKinteractor = useInteractor;
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
	if (vrepScene->startVisionSensorThread()) {
		camThread = std::thread(handleFunc, this);
	}else{
		camThread.~thread();
	}
	
	while (true) {
		updatePose();
		synchronizeDevices();
		
		chrono->increment();
		scale = (float)vr_renderWindowInteractor->GetPhysicalScale();
	
		if (!useInteractor) {
			vr_renderWindowInteractor->SetPhysicalScale(1);
		}
		vr_renderWindowInteractor->DoOneEvent(renderWindow, renderer); // render
		dynamicAddObjects();
		updateText();
		path->update();
		if (isReady()) {
			syncData();
			//grid->updatMap();
			chrono->increment2();
			vr_renderWindowInteractor->DoOneEvent(renderWindow, renderer); // render
			chrono->increment();
			setNotReady();
		}
		
		if (simxGetLastCmdTime(clientID) <= 0) {
			vr_renderWindowInteractor->TerminateApp(); // stop if the v-rep simulation is not running
			camThread.~thread();
			return;
		}
	}
}

