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



class eventCatcher : public vtkOpenVRInteractorStyle
{
public:
	eventCatcher() {
		SetDollyMotionFactor(10.0);
	}
	void OnButton3D(vtkEventData *edata) {
		int mode;
		bool trackpad = false;
		std::string signalName;
		vtkEventDataDeviceInput button = edata->GetAsEventDataDevice3D()->GetInput();
		vtkEventDataDevice controller = edata->GetAsEventDataDevice3D()->GetDevice();
		vtkEventDataAction action = edata->GetAsEventDataDevice3D()->GetAction();
		if (controller == vtkEventDataDevice::LeftController) {
			signalName = "LeftController";
		}
		else if (controller == vtkEventDataDevice::RightController) {
			signalName = "RightController";
		}

		if (button == vtkEventDataDeviceInput::Trigger) {
			signalName.append("Trigger");
		}

		if (action == vtkEventDataAction::Press) {
			mode = 1;
		}
		else if (action == vtkEventDataAction::Release) {
			mode = 0;
		}

		if ((controller == vtkEventDataDevice::LeftController) && (button == vtkEventDataDeviceInput::TrackPad)&& (action == vtkEventDataAction::Release)) {
			if (grid != nullptr){
				//cout << endl << "Collormap toggle" << endl;
				grid->toggleMode();
			}
		}
		simxSetIntegerSignal(clientID, signalName.c_str(), mode, simx_opmode_oneshot);
		vtkOpenVRInteractorStyle::OnButton3D(edata);
	}
	int clientID;
	vrep_volume_grid *grid;
protected:
};

void handleFunc(vr_renderwindow_support *sup) {
	cout << "Vision sensor thread activated" << endl;
	sup->visionSensorThread();
}

vr_renderwindow_support::vr_renderwindow_support(int cid) 
{ 
	clientID = cid; 
	chrono->coverage = &coverage; 
	chrono->scale = &scale; 
	// Now add spectator camera
	simxFloat *data;
	simxInt dataLength;
	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"checkForCam"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	if (data[0] != 0) {
		screenCam = new vrep_vision_sensor;
		screenCam->setClientID(clientID);
		float cameraParameters[] = { data[1], data[2], data[3], data[4], data[5] };
		screenCam->setCameraParams(data[0], cameraParameters);
		screenCamInUse = true;
		cout << "Detected screen capture camera (seriously decreases performance)" << endl;
	}
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
		if (screenCamInUse) {
			screenCam->getRenderer()->AddActor(vrepScene->getActor(i));
		}
	}

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

void vr_renderwindow_support::updatePose() {
	vrepScene->updateMainCamObjectPose();
}

void vr_renderwindow_support::syncData(vtkSmartPointer<vtkOpenVRRenderWindowInteractor> iren, vtkSmartPointer<vtkOpenVRRenderWindow> win, vtkSmartPointer<vtkOpenVRRenderer> ren) {
	//vrepScene->transferVisionSensorData(iren, win, ren);
	vrepScene->transferVisionSensorData();
}

void vr_renderwindow_support::visionSensorThread() {
	vrepScene->activateNewConnection(); // connect to vrep with a new port
	while (true) {
		vrepScene->updateVisionSensorObjectPose();
		coverage = vrepScene->updateVisionSensorRender();
		if (screenCamInUse) {
			screenCam->updateRender();
		}
		dataReady = true;
		path->update();
		//cout << "vision" << endl;
		while (dataReady) {
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
		};
	}
}

void vr_renderwindow_support::discoverDevices() {
	controller1_vrep = new vrep_controlled_object(clientID, -1);
	controller2_vrep = new vrep_controlled_object(clientID, -1);
	headset_vrep = new vrep_controlled_object(clientID, -1);
	
	headset_vrep->setName("Headset");
	headset_vrep->setDevice(vtkEventDataDevice::HeadMountedDisplay);		

	controller1_vrep->setName("Controller1");
	controller1_vrep->setDevice(vtkEventDataDevice::RightController);
		
	controller2_vrep->setName("Controller2");
	controller2_vrep->setDevice(vtkEventDataDevice::LeftController);

	//vtkSmartPointer<vtkOpenVRModel> temp;
	//for (int i = 1; i < 10; i++) {
	//	temp = renderWindow->GetTrackedDeviceModel(i);
	//	if (temp != nullptr) {
	//		cout << temp->GetName() << endl;
	//	}
	//}
}

void vr_renderwindow_support::synchronizeDevices() {
	headset_vrep->updatePosition(renderWindow, vr_renderWindowInteractor);
	controller1_vrep->updatePosition(renderWindow, vr_renderWindowInteractor);
	controller2_vrep->updatePosition(renderWindow, vr_renderWindowInteractor);
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
	renderer->SetBackground(1.0, 1.0, 1.0);
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
	if ((grid != nullptr) & (vrepScene->isVolumePresent())){
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
		vr_renderWindowInteractor->DoOneEvent(renderWindow, renderer); // render
		chrono->increment();
		updateText();
		scale = (float)vr_renderWindowInteractor->GetPhysicalScale();
		if (isReady()) {
			syncData(vr_renderWindowInteractor, renderWindow, renderer);
			//grid->updatMap();
			chrono->increment2();
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

