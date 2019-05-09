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

#include "stereoPanorama_renderwindow_support.h"
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
#include <vtkOpenGLPolyDataMapper.h>

#include <vtkOutputWindow.h>
#include <vtkFileOutputWindow.h>
#include <vtkPNGWriter.h>

void handleFunc(stereoPanorama_renderwindow_support *sup) {
	cout << "Vision sensor thread activated" << endl;
	sup->visionSensorThread();
}

stereoPanorama_renderwindow_support::stereoPanorama_renderwindow_support(int cid, int ref, int interactor)
{
	refH = ref;
	setClientID(cid, interactor);
	chrono->coverage = &coverage;
	// Now add spectator camera
	simxFloat *data;
	simxInt dataLength;

	simxGetObjectHandle(clientID, (simxChar*)"Vision_sensor", &handle, simx_opmode_blocking);

	vtkOutputWindow* ow = vtkOutputWindow::GetInstance();
	vtkFileOutputWindow* fow = vtkFileOutputWindow::New();
	fow->SetFileName("debug.log");
	if (ow)
	{
		ow->SetInstance(fow);
	}
	fow->Delete();
}

stereoPanorama_renderwindow_support::~stereoPanorama_renderwindow_support()
{

}


void stereoPanorama_renderwindow_support::addVrepScene(vrep_scene_content *vrepSceneIn) {
	vrepScene = vrepSceneIn;

	vrepScene->vrep_get_object_pose();
	for (int i = 0; i < 1; i++) { //vrepScene->getNumActors()
		vrepScene->getActor(i)->PickableOff();
		vrepScene->getActor(i)->GetProperty()->SetAmbient(0.7);
		vrepScene->getActor(i)->GetProperty()->SetDiffuse(0.5);
	}
	if (vrepScene->isVolumePresent()) {
		grid = vrepScene->vol;
		//renderer->ResetCamera();
	}
	else {
		grid = nullptr;
	}
	grid = nullptr;
}

void stereoPanorama_renderwindow_support::updatePose() {
	vrepScene->updateMainCamObjectPose();

	simxFloat eulerAngles[3];
	simxFloat position[3];

	simxGetObjectOrientation(clientID, handle, refH, eulerAngles, simx_opmode_streaming); // later replace by : simx_opmode_buffer 
	simxGetObjectPosition(clientID, handle, refH, position, simx_opmode_streaming);
}

void stereoPanorama_renderwindow_support::syncData() {
	//vrepScene->transferVisionSensorData(iren, win, ren);
	vrepScene->transferVisionSensorData();
}

void stereoPanorama_renderwindow_support::visionSensorThread() {
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




void stereoPanorama_renderwindow_support::activate_interactor() {
	vrepScene->vrep_get_object_pose();

	activateMainCam(); // shader that combines images

	if ((grid != nullptr) && (vrepScene->isVolumePresent())) {
		cout << "Volume detected and connected" << endl;
	}

	// Manage vision sensor thread (if necessary)
	std::thread camThread;
	if (vrepScene->startVisionSensorThread()) {
		camThread = std::thread(handleFunc, this);
	}
	else {
		camThread.~thread();
	}
	while (true) {
		vtkSmartPointer<vtkImageAppend> vertical = vtkSmartPointer<vtkImageAppend>::New();
		//vertical->SetAppendAxis()
		//vertical->SetAppendAxis(1);
		vtkSmartPointer<vtkExtractVOI> extractVOI =vtkSmartPointer<vtkExtractVOI>::New();
		for (int i = 0; i < 1000; i++) {
			vtkSmartPointer<vtkImageData> im;
			renderWindow->Render();
			filter->Modified();
			filter->ReadFrontBufferOff();
			filter->Update();
			extractVOI->SetInputConnection(filter->GetOutputPort());
			vertical->AddInputData(extractVOI->GetOutput());
		}
		cout << vertical->GetNumberOfInputs() << endl;
		vtkSmartPointer<vtkPNGWriter> wr = vtkSmartPointer<vtkPNGWriter>::New();
		wr->SetFileName("test.png");
		wr->SetInputConnection(vertical->GetOutputPort());
		wr->Write();
		chrono->increment();

		path->update();
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

void stereoPanorama_renderwindow_support::activateMainCam() {
	vr_camera->SetViewAngle(90.0);
	vr_camera->SetPosition(0, 0, 0);
	vr_camera->SetFocalPoint(0, 0, 1);
	vr_camera->SetViewUp(0, 1, 0);

	// Classical vtk pipeline to set up renderwindow etc with extra options
	renderer->SetActiveCamera(vr_camera);
	renderer->UseShadowsOff();
	renderer->Modified();

	renderWindow->AddRenderer(renderer);
	//renderWindow->SetDesiredUpdateRate(90.0);
	renderWindow->SetSize(121, 768);
	renderWindow->SetOffScreenRendering(true);
	renderWindow->Initialize();
	vr_renderWindowInteractor->SetRenderWindow(renderWindow);
	vr_renderWindowInteractor->Initialize();
	filter->SetInput(renderWindow);
}



