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
#include <math.h>      
#include <ppl.h>

#include <vtkOutputWindow.h>
#include <vtkFileOutputWindow.h>
#include <vtkPNGWriter.h>

#define PI 3.14159265

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

	simxGetObjectHandle(clientID, (simxChar*)"Stereo_omnidirectional_camera", &handle, simx_opmode_blocking);

	vtkOutputWindow* ow = vtkOutputWindow::GetInstance();
	vtkFileOutputWindow* fow = vtkFileOutputWindow::New();
	fow->SetFileName("debug.log");
	if (ow)
	{
		ow->SetInstance(fow);
	}
	fow->Delete();

	// for multiThreading
	for (int i = 0; i < 4; i++) {
		renderer.push_back(vtkSmartPointer<vtkOpenGLRenderer>::New());
		renderWindow.push_back(vtkSmartPointer<vtkWin32OpenGLRenderWindow>::New());
		vr_renderWindowInteractor.push_back(vtkSmartPointer<vtkWin32RenderWindowInteractor>::New());
		vr_camera.push_back(vtkSmartPointer<vtkOpenGLCamera>::New());
		pose.push_back(vtkSmartPointer<vtkTransform>::New());
		filter.push_back(vtkSmartPointer<vtkWindowToImageFilter>::New());
		slice.push_back(vtkSmartPointer<vtkImageData>::New());
	}
}

stereoPanorama_renderwindow_support::~stereoPanorama_renderwindow_support()
{

}

void stereoPanorama_renderwindow_support::addVrepScene(vrep_scene_content *vrepSceneIn) {
	vrepScene = vrepSceneIn;

	vrepScene->vrep_get_object_pose();
	for (int k = 0; k < vr_camera.size(); k++) {
		for (int i = 0; i < vrepScene->getNumActors(); i++) {
			vrepScene->getActor(i)->PickableOff();
			vrepScene->getActor(i)->GetProperty()->SetAmbient(0.6);
			vrepScene->getActor(i)->GetProperty()->SetDiffuse(0.4);
			vrepScene->getActor(i)->GetProperty()->SetSpecular(0.5);
			vrepScene->getActor(i)->GetProperty()->SetSpecularColor(0.25, 0.25, 0.25);
			renderer[k]->AddActor(vrepScene->getActor(i));
			if (k == 0) {
				visibilityLayer.push_back((std::bitset<16>)vrepScene->getVisibilityLayer(i));
			}
		}
		lastObject = vrepScene->getNumActors();
		for (int i = 0; i < vrepScene->getNumRenders(); i++) {
			renderer[k]->AddActor(vrepScene->getPanelActor(i));
		}
		if (vrepScene->isVolumePresent()) {
			renderer[k]->AddViewProp(vrepScene->getVolume());
			grid = vrepScene->vol;
			//renderer->ResetCamera();
		}
		else {
			grid = nullptr;
		}
	
	}
	
}

void stereoPanorama_renderwindow_support::checkLayers() {
	simxInt val;
	simxGetIntegerSignal(clientID, (simxChar*)"VisibleLayers", &val, simx_opmode_streaming);
	for (int i = 0; i < vrepScene->getNumActors(); i++) {
		std::bitset<16> mybit = (std::bitset<16>)val;
		mybit = (mybit & visibilityLayer[i]) & visibilityLayer[i];
		vrepScene->getActor(i)->SetVisibility(mybit.any());
	}
}

void stereoPanorama_renderwindow_support::updatePose() {
	vrepScene->updateMainCamObjectPose();

	simxFloat eulerAngles[3];
	simxFloat position[3];
	simxGetObjectOrientation(clientID, handle, refH, eulerAngles, simx_opmode_streaming); // later replace by : simx_opmode_buffer 
	simxGetObjectPosition(clientID, handle, refH, position, simx_opmode_streaming);
	for (int k = 0; k < vr_camera.size(); k++) {
		pose[k]->PostMultiply();
		pose[k]->Identity();
		pose[k]->RotateZ((eulerAngles[2] * 180 / 3.1415));
		pose[k]->RotateY((eulerAngles[1] * 180 / 3.1415));
		pose[k]->RotateX((eulerAngles[0] * 180 / 3.1415));

		pose[k]->Translate(position);
		pose[k]->RotateX(-90);
		pose[k]->Inverse();
		pose[k]->Modified();
	}
}

void stereoPanorama_renderwindow_support::readLights() {
	simxInt *dataInt;
	simxFloat *dataFloat;
	simxInt dataFloatLength;
	simxInt dataIntLength;

	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"getLightInfo"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &dataIntLength, &dataInt, &dataFloatLength, &dataFloat, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	vtkSmartPointer<vtkLight> light;
	vtkSmartPointer<vtkTransform> tform;
	for (int k = 0; k < vr_camera.size(); k++) {
		for (int i = 0; i < dataInt[0]; i++) {
			light = vtkSmartPointer<vtkLight>::New();
			tform = vtkSmartPointer<vtkTransform>::New();

			tform->PostMultiply();
			tform->Identity();
			tform->RotateZ((dataFloat[(i * 12) + 5] * 180 / 3.1415));
			tform->RotateY((dataFloat[(i * 12) + 4] * 180 / 3.1415));
			tform->RotateX((dataFloat[(i * 12) + 3] * 180 / 3.1415));
			tform->Translate(dataFloat[(i * 12)], dataFloat[(i * 12) + 1], dataFloat[(i * 12) + 2]);
			tform->RotateX(-90);
			tform->Modified();
			light->SetPosition(0, 0, 0);
			light->SetFocalPoint(0, 0, 1);

			light->SetDiffuseColor(dataFloat[(i * 12) + 6], dataFloat[(i * 12) + 7], dataFloat[(i * 12) + 8]);
			light->SetSpecularColor(dataFloat[(i * 12) + 9], dataFloat[(i * 12) + 10], dataFloat[(i * 12) + 11]);
			light->SetFocalPoint(0, 0, 1);
			light->SetConeAngle(180.0);
			light->SetTransformMatrix(tform->GetMatrix());
			renderer[k]->AddLight(light);
		}
	}
}

void stereoPanorama_renderwindow_support::dynamicAddObjects() {
	int result;
	simxGetIntegerSignal(clientID, "dynamic_load_request", &result, simx_opmode_streaming);
	if (result == 0) {
		return;
	}
	vrepScene->dynamicLoad();
	for (int k = 0; k < vr_camera.size(); k++) {
		for (int i = lastObject; i < vrepScene->getNumActors(); i++) {
			vrepScene->getActor(i)->PickableOff();
			vrepScene->getActor(i)->GetProperty()->SetAmbient(0.7);
			vrepScene->getActor(i)->GetProperty()->SetDiffuse(0.5);
			renderer[k]->AddActor(vrepScene->getActor(i));
		}
	}
	lastObject = vrepScene->getNumActors();
	simxSetIntegerSignal(clientID, "dynamic_load_request", 0, simx_opmode_oneshot);
}

void stereoPanorama_renderwindow_support::syncData() {
	//vrepScene->transferVisionSensorData(iren, win, ren);
	vrepScene->transferVisionSensorData();
}

void stereoPanorama_renderwindow_support::visionSensorThread() {
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


void stereoPanorama_renderwindow_support::renderStrip(float dist, bool left, bool top, int k) {
	vtkSmartPointer<vtkImageAppend> horizontal = vtkSmartPointer<vtkImageAppend>::New();
	vtkSmartPointer<vtkExtractVOI> extractVOI;
	vtkSmartPointer<vtkTransform> prePose = vtkSmartPointer<vtkTransform>::New();

	int width = 4096;
	int temp = 1;
	width = width / temp;
	if (left) {
		dist = -dist;
	}
	float angle = -45;
	if (top) {
		angle = 45;
	}
	for (int i = 0; i < width; i++) {
		extractVOI = vtkSmartPointer<vtkExtractVOI>::New();
		prePose->Identity();
		prePose->Translate(dist, 0.0, 0.0);
		prePose->RotateX(angle);
		prePose->RotateY(180 + 360.0*(float)i / (float)width);
		
		prePose->PreMultiply();
		pose[k]->PreMultiply();
		prePose->Concatenate(pose[k]);
		vr_camera[k]->SetModelTransformMatrix(prePose->GetMatrix());
		vr_camera[k]->Modified();
		
		vr_camera[k]->Modified();
		renderer[k]->Render();
		filter[k]->Modified();
		filter[k]->ReadFrontBufferOff();
		filter[k]->Update();
		extractVOI->SetInputConnection(filter[k]->GetOutputPort());
		extractVOI->SetVOI(60, 60, 0, 767, 0, 0);
		extractVOI->Update();
		locked = false;
		horizontal->AddInputData(extractVOI->GetOutput());
	}
	horizontal->Update();
	slice[k] = horizontal->GetOutput();
}

void stereoPanorama_renderwindow_support::activate_interactor() {
	vrepScene->vrep_get_object_pose();
	updatePose();
	activateMainCam(); // shader that combines images
	readLights();

	if ((grid != nullptr) && (vrepScene->isVolumePresent())) {
		cout << "Volume detected and connected" << endl;
	}

	// Search for dynamic path object
	path = new pathObject(clientID);
	for (int k = 0; k < vr_camera.size(); k++) {
		renderer[k]->AddActor(path->getActor());
		renderer[k]->Modified();
	}

	// Manage vision sensor thread (if necessary)
	std::thread camThread;
	bool threadActivated = false;
	if (vrepScene->startVisionSensorThread()) {
		busy = true;
		camThread = std::thread(handleFunc, this);
		threadActivated = true;
	}
	else {
		camThread.~thread();
	}


	while (true) {
		updatePose();
		path->update();
		//checkLayers();

		for (int k = 0; k < vr_camera.size(); k++) {
			renderWindow[k]->Render();
		}
		vtkSmartPointer<vtkImageAppend> vertical = vtkSmartPointer<vtkImageAppend>::New();
		vertical->SetAppendAxis(1);
		cout << "Started rendering image" << endl;
		clock_t begin = clock();
		// SPEED!!!!
	/*	concurrency::parallel_invoke(
		[&] { renderStrip(0.1, true, false, 0); },
		[&] { renderStrip(0.1, true, true, 1); },
		[&] { renderStrip(0.1, false, false, 2); }, 
		[&] { renderStrip(0.1, false, true, 3); }
		);*/

		renderStrip(0.035, false, false, 0);
		renderStrip(0.035, false, true, 1);
		renderStrip(0.035, true, false, 2);
		renderStrip(0.035, true, true, 3);

		for (int i = 0; i < 4; i++) {
			vertical->AddInputData(slice[i]); // top left
		}
		vtkSmartPointer<vtkPNGWriter> wr = vtkSmartPointer<vtkPNGWriter>::New();
		wr->SetFileName("test.png");
		wr->SetInputConnection(vertical->GetOutputPort());
		wr->Write();

		clock_t end = clock();
		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		cout << "Finished rendering image in " << elapsed_secs << " seconds" << endl;
		if (isReady()) {
			syncData();
			//grid->updatMap();
			chrono->increment2();
			setNotReady();
		}

		if (simxGetLastCmdTime(clientID) <= 0) {
			for (int k = 0; k < vr_camera.size(); k++) {
				vr_renderWindowInteractor[k]->TerminateApp(); // stop if the v-rep simulation is not running
			}
			camThread.~thread();
			return;
		}
	}
}

void stereoPanorama_renderwindow_support::activateMainCam() {
	for (int k = 0; k < vr_camera.size(); k++) {
		vr_camera[k]->SetViewAngle(90.0);
		vr_camera[k]->SetPosition(0, 0, 0);
		vr_camera[k]->SetFocalPoint(0, 0, 1);
		vr_camera[k]->SetViewUp(0, 1, 0);
		vr_camera[k]->SetModelTransformMatrix(pose[k]->GetMatrix());
		// Classical vtk pipeline to set up renderwindow etc with extra options
		renderer[k]->SetActiveCamera(vr_camera[k]);
		renderer[k]->UseShadowsOff();
		renderer[k]->Modified();
		renderer[k]->LightFollowCameraOff();
		renderer[k]->AutomaticLightCreationOff();
		renderWindow[k]->AddRenderer(renderer[k]);
		renderWindow[k]->SetSize(121.0, 768.0); // make shure we have a 'middle' pixel
		renderWindow[k]->SetOffScreenRendering(true);
		renderWindow[k]->Initialize();
		renderWindow[k]->SetDesiredUpdateRate(1000.0);
		vr_renderWindowInteractor[k]->SetRenderWindow(renderWindow[k]);
		vr_renderWindowInteractor[k]->Initialize();
		filter[k]->SetInput(renderWindow[k]);
		filter[k]->ShouldRerenderOff();
	}
}



