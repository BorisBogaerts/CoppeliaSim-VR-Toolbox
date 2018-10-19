#include "vr_renderwindow_support.h"
#include <vector>
#include "extApi.h"
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>
#include <vtkOpenVRModel.h>

#include <vtkOpenVRInteractorStyle.h>
#include <thread>
#include <chrono>
#include <vtkOpenVRModel.h>
#include <vtkSkybox.h>
#include <vtkOpenVRPanelRepresentation.h>
#include <vtkOpenVRPanelWidget.h>

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
			if (grid != nullptr) {
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

	//vrepScene->vrep_get_object_pose();
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
		if (screenCamInUse) {
			//screenCam->getRenderer()->AddActor(vrepScene->getPanelActor(i));
		}
	}

	if (vrepScene->isVolumePresent()) {
		renderer->AddViewProp(vrepScene->getVolume());
		if (screenCamInUse) {
			//screenCam->getRenderer()->AddViewProp(vrepScene->getVolume());
		}
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

void vr_renderwindow_support::activate_interactor() {
	vrepScene->vrep_get_object_pose();
	renderer->SetActiveCamera(vr_camera);
	renderer->UseFXAAOn();
	renderer->SetBackground(1.0, 1.0, 1.0);
	renderer->AutomaticLightCreationOn();

	renderWindow->AddRenderer(renderer);
	renderWindow->SetMultiSamples(0);
	vr_renderWindowInteractor->SetRenderWindow(renderWindow);
	renderer->SetAutomaticLightCreation(true);
	renderer->LightFollowCameraOn();
	
	if (screenCamInUse) {
		screenCam->getRenderer()->AutomaticLightCreationOn();
		screenCam->getRenderer()->LightFollowCameraOn();
		screenCam->getRenderer()->Modified();
		screenCam->activateBasic();
		renderer->RemoveAllViewProps();
	}
	eventCatcher *events = new(eventCatcher); // define object that captures buttonpress
	events->clientID = clientID;
	if (grid != nullptr) {
		events->grid = grid;
	}
	vr_renderWindowInteractor->SetInteractorStyle(events); // set object which captures buttonpress
	renderer->UseShadowsOff();
	renderer->Modified();
	renderWindow->SetDesiredUpdateRate(90.0);

	cout << "Everithing loaded succesfully" << endl;
	renderWindow->Initialize();
	vr_renderWindowInteractor->Initialize();
	renderWindow->Render();
	vr_renderWindowInteractor->Enable();
	vr_renderWindowInteractor->Render();

	discoverDevices();
	std::thread camThread;
	if (vrepScene->startVisionSensorThread()) {
		camThread = std::thread(handleFunc, this);
	}else{
		camThread.~thread();
	}
	
	while (true) {
		updatePose();
		vr_renderWindowInteractor->DoOneEvent(renderWindow, renderer); // render
		synchronizeDevices();
		chrono->increment();
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

