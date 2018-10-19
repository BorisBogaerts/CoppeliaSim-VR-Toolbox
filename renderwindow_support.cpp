#pragma once

#include "renderwindow_support.h"
#include <vector>
#include "extApi.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <thread>
#include <chrono>


void ClickCallbackFunction(vtkObject* vtkNotUsed(caller), long unsigned int vtkNotUsed(eventId), void* in, void* vtkNotUsed(callData))
{
	renderwindow_support* temp = static_cast<renderwindow_support *>(in);
	temp->updatePose();
	temp->updateRender();
	temp->chrono->increment();
	if (temp->isReady()) {
		temp->syncData();
		temp->setNotReady();
	}
}

renderwindow_support::~renderwindow_support()
{
	
}


void renderwindow_support::updateRender() {
	vr_renderWindowInteractor->Render();
};

void renderwindow_support::updatePose() {
	vrepScene->updateMainCamObjectPose();
}

void renderwindow_support::syncData() {
	vrepScene->transferVisionSensorData();
}


void renderwindow_support::addVrepScene(vrep_scene_content *vrepSceneIn) {
	vrepScene = vrepSceneIn;
	vrepScene->vrep_get_object_pose();
	for (int i = 0; i < vrepScene->getNumActors(); i++) {
		vrepScene->getActor(i)->PickableOff();
		renderer->AddActor(vrepScene->getActor(i));
	}

	for (int i = 0; i < vrepScene->getNumRenders(); i++)
		renderer->AddActor(vrepScene->getPanelActor(i));

	if (vrepScene->isVolumePresent()) {
		renderer->AddViewProp(vrepScene->getVolume());
	}
	//renderer->ResetCamera();
	//vrepScene->getVolume()->Print(cout);
}

void renderwindow_support::visionSensorThread() {
	vrepScene->activateNewConnection(); // connect to vrep with a new port
	while (true) {
		vrepScene->updateVisionSensorObjectPose();
		vrepScene->updateVisionSensorRender();
		dataReady = true;
		while (dataReady) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		};
	}
}

void handleFunc(renderwindow_support *sup) {
	cout << "Vision sensor thread activated" << endl;
	sup->visionSensorThread();
}

void renderwindow_support::activate_interactor() {
	// add actors
	
	renderer->SetBackground(1.0, 1.0, 1.0);
	renderer->SetActiveCamera(vr_camera);
	double range[]  = { 0.01,100 };
	vr_camera->SetClippingRange(range);
	
	renderWindow->AddRenderer(renderer);
	renderWindow->SetDesiredUpdateRate(100);
	//renderWindow->SetDesiredUpdateRate(update);
	vr_renderWindowInteractor->SetRenderWindow(renderWindow);
	
	vrepScene->vrep_get_object_pose();
	
	vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
	vr_renderWindowInteractor->SetInteractorStyle(style);
	vr_renderWindowInteractor->Initialize();
	// Callback
	
	vtkSmartPointer<vtkCallbackCommand> clickCallback =
		vtkSmartPointer<vtkCallbackCommand>::New();
	clickCallback->SetCallback(ClickCallbackFunction);
	clickCallback->SetClientData(this);
	
	vr_renderWindowInteractor->AddObserver(vtkCommand::TimerEvent, clickCallback);
	vr_renderWindowInteractor->CreateRepeatingTimer(5);
	
	std::thread camThread(handleFunc, this);

	//this->updatePose();
	vr_renderWindowInteractor->Start();
}


