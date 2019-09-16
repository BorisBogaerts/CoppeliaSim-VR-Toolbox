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

#pragma once

#include "renderwindow_support.h"
#include <vector>
#include "extApi.h"
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>
#include <thread>
#include <chrono>
#include <ppl.h>
#include "vrep_plot_container.h"

void handleFunc(renderwindow_support *sup) {
	cout << "Vision sensor thread activated" << endl;
	sup->visionSensorThread();
}

renderwindow_support::renderwindow_support(int cid, int ref, int interactor)
{
	refH = ref;
	setClientID(cid, interactor);
	chrono->coverage = &coverage;
	simxGetObjectHandle(clientID, (simxChar*)"Vision_sensor", &handle, simx_opmode_blocking);
}

renderwindow_support::~renderwindow_support()
{

}

void renderwindow_support::updateRender() {
	vr_renderWindowInteractor->Render();
};


void renderwindow_support::addVrepScene(vrep_scene_content *vrepSceneIn) {
	vrepScene = vrepSceneIn;

	vrepScene->vrep_get_object_pose();
	for (int i = 0; i < vrepScene->getNumActors(); i++) {
		vrepScene->getActor(i)->PickableOff();
		vrepScene->getActor(i)->GetProperty()->SetAmbient(0.7);
		vrepScene->getActor(i)->GetProperty()->SetDiffuse(0.5);
		renderer->AddActor(vrepScene->getActor(i));
	}
	//for (int i = 0; i < vrepScene->getNumRenders(); i++) {
	//	for (int j = 0; j < renderer.size(); j++) {
	//		renderer[j]->AddActor(vrepScene->getPanelActor(i));
	//	}
	//}
	if (vrepScene->isVolumePresent()) {
		renderer->AddViewProp(vrepScene->getVolume());
		grid = vrepScene->vol;
		//renderer->ResetCamera();
	}
	else {
		grid = nullptr;
	}
}

void renderwindow_support::updatePose() {
	vrepScene->updateMainCamObjectPose();

	simxFloat eulerAngles[3];
	simxFloat position[3];

	simxGetObjectOrientation(clientID, handle, refH, eulerAngles, simx_opmode_streaming); // later replace by : simx_opmode_buffer 
	simxGetObjectPosition(clientID, handle, refH, position, simx_opmode_streaming);
	pose->PostMultiply();
	pose->Identity();
	pose->RotateZ((eulerAngles[2] * 180 / 3.1415));
	pose->RotateY((eulerAngles[1] * 180 / 3.1415));
	pose->RotateX((eulerAngles[0] * 180 / 3.1415));

	pose->Translate(position);
	pose->RotateX(-90);
	pose->Inverse();
	pose->Modified();
	vr_camera->SetModelTransformMatrix(pose->GetMatrix());
	vr_camera->Modified();
}

void renderwindow_support::syncData() {
	//vrepScene->transferVisionSensorData(iren, win, ren);
	vrepScene->transferVisionSensorData();
}

void renderwindow_support::visionSensorThread() {
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




void renderwindow_support::activate_interactor() {
	vrepScene->vrep_get_object_pose();

	vr_camera->SetViewAngle(90.0);
	vr_camera->SetModelTransformMatrix(pose->GetMatrix());
	vr_camera->SetPosition(0, 0, 0);
	vr_camera->SetFocalPoint(0, 0, 1);
	vr_camera->SetViewUp(0, 1, 0);

	// Classical vtk pipeline to set up renderwindow etc with extra options
	renderer->SetActiveCamera(vr_camera);
	
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
	renderWindow->SetDesiredUpdateRate(90.0);
	renderWindow->SetSize(1024, 1024);
	renderWindow->Initialize();
	vr_renderWindowInteractor->SetRenderWindow(renderWindow);
	vr_renderWindowInteractor->Initialize();


	if ((grid != nullptr) && (vrepScene->isVolumePresent())) {
		cout << "Volume detected and connected" << endl;
	}

	cout << "Everithing loaded succesfully" << endl;


	// Search for dynamic path object
	path = new pathObject(clientID);
	renderer->AddActor(path->getActor());
	renderer->Modified();

	// Create plot container
	vrep_plot_container *cont = new vrep_plot_container(clientID, renderer);

	// Manage vision sensor thread (if necessary)
	std::thread camThread;
	if (vrepScene->startVisionSensorThread()) {
		camThread = std::thread(handleFunc, this);
	}
	else {
		camThread.~thread();
	}
	while (true) {
		updatePose();
		updateRender();
		chrono->increment();
		
		path->update();

		// add plot content
		cont->update();

		if (isReady()) {
			syncData();
			//grid->updatMap();
			chrono->increment2();
			setNotReady();
		}

		if (simxGetLastCmdTime(clientID) <= 0) {
			vr_renderWindowInteractor->TerminateApp(); // stop if the v-rep simulation is not running
			camThread.~thread();
			return;
		}
	}
}

