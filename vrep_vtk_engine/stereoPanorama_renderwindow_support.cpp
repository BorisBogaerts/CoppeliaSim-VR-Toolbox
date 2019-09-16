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
#include <vtkVolumeProperty.h>
#include <math.h>      
#include <ppl.h>

#include <vtkOutputWindow.h>
#include <vtkFileOutputWindow.h>
#include <vtkPNGWriter.h>
#include <vtkPNGReader.h>
#include "vrep_plot_container.h"
#include <vtkAlgorithmOutput.h>

#define PI 3.14159265



stereoPanorama_renderwindow_support::stereoPanorama_renderwindow_support(int cid, int ref, int interactor)
{
	refH = ref;
	setClientID(cid, interactor);
	chrono->coverage = &coverage;
	// Now add spectator camera
	simxFloat *data;
	simxInt dataLength;

	simxGetObjectHandle(clientID, (simxChar*)"VR360_cam", &handle, simx_opmode_blocking);

	vtkOutputWindow* ow = vtkOutputWindow::GetInstance();
	vtkFileOutputWindow* fow = vtkFileOutputWindow::New();
	fow->SetFileName("VR360_debug.log");
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
			if (k == 0) {
				renderer[k]->AddActor(vrepScene->getActor(i));
			}
			else {
				renderer[k]->AddActor(vrepScene->getNewActor(i));
			}
			if (k == 0) {
				visibilityLayer.push_back((std::bitset<16>)vrepScene->getVisibilityLayer(i));
			}
		}
		lastObject = vrepScene->getNumActors();
			for (int i = 0; i < vrepScene->getNumRenders(); i++) {
				if (k == 0) {
					renderer[k]->AddActor(vrepScene->getPanelActor(i));
				}
				else {
					renderer[k]->AddActor(vrepScene->getNewPanelActor(i));
				}
			}
			if (vrepScene->isVolumePresent()) {
				if (k == 0) {
					renderer[k]->AddVolume(vrepScene->getVolume());
				}
				else {
					renderer[k]->AddVolume(vrepScene->getNawVolume());
				}
				grid = vrepScene->vol;
				//renderer[k]->ResetCamera();
			}
			else {
				grid = nullptr;
			}
			for (int i = 0; i < vrepScene->getNumberOfLights(); i++) {
				if (k == 0) {
					renderer[k]->AddLight(vrepScene->getLight(i));
				}
				else {
					renderer[k]->AddLight(vrepScene->getNewLight(i));
				}
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


void stereoPanorama_renderwindow_support::dynamicAddObjects() {
	int result;
	simxGetIntegerSignal(clientID, "dynamic_load_request", &result, simx_opmode_streaming);
	if (result == 0) {
		return;
	}
	vrepScene->dynamicLoad();
	for (int k = 0; k < 1; k++) {
		for (int i = lastObject; i < vrepScene->getNumActors(); i++) {
			vtkSmartPointer<vtkActor> temp;
			if (k == 0) {
				temp = vrepScene->getActor(i);
			}
			else {
				temp = vrepScene->getNewActor(i);
			}
			temp->PickableOff();
			temp->GetProperty()->SetAmbient(0.7);
			temp->GetProperty()->SetDiffuse(0.5);
			renderer[k]->AddActor(temp);
		}
	}
	lastObject = vrepScene->getNumActors();
	simxSetIntegerSignal(clientID, "dynamic_load_request", 0, simx_opmode_oneshot);
}

void stereoPanorama_renderwindow_support::syncData() {
	//vrepScene->transferVisionSensorData(iren, win, ren);
	vrepScene->transferVisionSensorData();
}

//vtkSmartPointer<vtkAlgorithmOutput> runFXAA(vtkSmartPointer<vtkImageData> im) {
//	vtkSmartPointer<vtkOpenGLRenderer> render = vtkSmartPointer<vtkOpenGLRenderer>::New();
//	vtkSmartPointer<vtkRenderWindow> renwin = vtkSmartPointer<vtkRenderWindow>::New();
//	vtkSmartPointer<vtkOpenVRRenderWindowInteractor> renwinI = vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New();
//
//	vtkSmartPointer<vtkOpenGLTexture> tex = vtkSmartPointer<vtkOpenGLTexture>::New();
//	vtkSmartPointer<vtkWindowToImageFilter> fi = vtkSmartPointer<vtkWindowToImageFilter>::New();
//	tex->SetInputData(im);
//	tex->Update();
//	render->SetTexturedBackground(true);
//	render->SetBackgroundTexture(tex);
//	render->UseFXAAOn();
//	renwin->AddRenderer(render);
//	renwin->SetSize(im->GetDimensions()[0], im->GetDimensions()[1]);
//	renwin->SetAlphaBitPlanes(1);
//	//renwin->SetOffScreenRendering(true);
//	
//	renwinI->SetRenderWindow(renwin);
//	fi->SetInput(renwin);
//	fi->SetInputBufferTypeToRGBA();
//	fi->ShouldRerenderOff();
//	cout << "Did this" << endl;
//	render->Render();
//	cout << "Did this" << endl;
//	fi->Modified();
//	fi->ReadFrontBufferOff();
//	fi->Update();
//	render->Delete();
//	renwin->Delete();
//	renwinI->Delete();
//	tex->Delete();
//	return fi->GetOutputPort();
//}



void stereoPanorama_renderwindow_support::renderStrip(float dist, bool left, bool top, int k, int width, int height) {
	vtkSmartPointer<vtkImageAppend> horizontal = vtkSmartPointer<vtkImageAppend>::New();
	vtkSmartPointer<vtkExtractVOI> extractVOI;
	vtkSmartPointer<vtkTransform> prePose = vtkSmartPointer<vtkTransform>::New();
	int temp = 1;
	width = width / temp;
	dist = dist / 2;
	if (left) {
		dist = -dist;
	}
	float angle = -45;
	if (top) {
		angle = 45;
	}
	vr_renderWindowInteractor[k]->Render();
	height = ((int)(height / 4)) - 1; // 0 based counting
	for (int i = 0; i < width; i++) {
		extractVOI = vtkSmartPointer<vtkExtractVOI>::New();
		prePose->Identity();
		prePose->Translate(dist, 0.0, 0.0);
		prePose->RotateX(angle);
		prePose->RotateY(180 + (360.0*(float)i / (float)(width-1))); // 0 based counting
		

		prePose->PreMultiply();
		pose[k]->PreMultiply();
		prePose->Concatenate(pose[k]);
		prePose->Modified();
		vr_camera[k]->SetModelTransformMatrix(prePose->GetMatrix());
		vr_camera[k]->Modified();
		renderer[k]->Render();
		filter[k]->Modified();
		filter[k]->ReadFrontBufferOff();
		filter[k]->Update();
		extractVOI->SetInputConnection(filter[k]->GetOutputPort());
		extractVOI->SetVOI(60, 60, 0, height, 0, 0);
		extractVOI->Update();
		locked = false;
		horizontal->AddInputData(extractVOI->GetOutput());
	}
	horizontal->Update();
	slice[k] = horizontal->GetOutput();
}

std::string ExePath() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

void stereoPanorama_renderwindow_support::activate_interactor() {
	// Get all necessary parameters
	simxInt *data;
	simxInt dataLength;
	simxCallScriptFunction(clientID, (simxChar*)"VR360_cam", sim_scripttype_childscript, (simxChar*)"getParams"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &dataLength, &data, 0, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	int core, width, height;
	core = data[0];
	height = data[1];
	width = data[2];

	// Standard things
	vrepScene->vrep_get_object_pose();
	updatePose();

	// Make renderers etc
	activateMainCam((int)(height/4)); // rendered in 4 seperate slices

	if ((grid != nullptr) && (vrepScene->isVolumePresent())) {
		cout << "Volume detected and connected" << endl;
	}

	// Search for dynamic path object
	path = new pathObject(clientID);
	for (int k = 0; k < vr_camera.size(); k++) {
		vtkSmartPointer<vtkActor> temp;
		if (k == 0) {
			temp = path->getActor();
		}
		else {
			temp = path->getNewActor();
		}
		renderer[k]->AddActor(temp);
		renderer[k]->Modified();
	}
	std::string fileName = ExePath();
	fileName.append("\\imageTransfer.png");
	//vrepScene->vol->toggleMode();
	cout << "Temporary file save location : " << fileName << endl;
	
	// Define plot container
	vrep_plot_container *pc = new vrep_plot_container(clientID, renderer);

	if (core == 1) {
		cout << "Multi core rendering activated" << endl;
	}else{
		cout << "Single core rendering activated" << endl;
	}
	while (true) {
		for (int i = 0; i < 2; i++) {
			if (vrepScene->startVisionSensorThread()) { // if vision sensor
				vrepScene->updateVisionSensorObjectPose();
				coverage = vrepScene->updateVisionSensorRender();
				syncData();
			}
			//checkBackground();
			updatePose();
			checkLayers();
			path->update();
			dynamicAddObjects(); // see if there are new objects in the scene, if so add them
		}
		int trigger;
		simxGetIntegerSignal(clientID, "TriggerVR360", &trigger,simx_opmode_blocking);
		if (trigger == 0) {
			continue; 
		}

		// update plot container
		pc->update();

		vtkSmartPointer<vtkImageAppend> vertical = vtkSmartPointer<vtkImageAppend>::New();
		vertical->SetAppendAxis(1);
		cout << "Started rendering image" << endl;
		clock_t begin = clock();
		
		float IPD;
		simxGetFloatSignal(clientID, "IPD", &IPD, simx_opmode_blocking);
		if (core == 1) {
			// SPEED!!!!
			concurrency::parallel_invoke(
				[&] { renderStrip(IPD, false, false, 0, width, height); },
				[&] { renderStrip(IPD, false, true, 1, width, height); },
				[&] { renderStrip(IPD, true, false, 2, width, height); },
				[&] { renderStrip(IPD, true, true, 3, width, height); }
			);
		}
		else {
			// No speed
			renderStrip(IPD, false, false, 0, width, height);
			renderStrip(IPD, false, true, 1, width, height);
			renderStrip(IPD, true, false, 2, width, height);
			renderStrip(IPD, true, true, 3, width, height);
		}
		for (int i = 0; i < 4; i++) {
			vertical->AddInputData(slice[i]); // top left
		}
		
		vtkSmartPointer<vtkPNGWriter> wr = vtkSmartPointer<vtkPNGWriter>::New();
		wr->SetFileName(fileName.c_str());
		vertical->Update();
		//wr->SetInputConnection(runFXAA(vertical->GetOutput()));
		wr->SetInputConnection(vertical->GetOutputPort());
		wr->Write();

		simxCallScriptFunction(clientID, (simxChar*)"VR360_cam", sim_scripttype_childscript, (simxChar*)"setVisionSensorImage"
			, 0, NULL, 0, NULL, fileName.size(), fileName.c_str(), 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);
		clock_t end = clock();
		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		cout << "Finished rendering image in " << elapsed_secs << " seconds (waiting for your next trigger)" << endl;

		if (simxGetLastCmdTime(clientID) <= 0) {
			for (int k = 0; k < vr_camera.size(); k++) {
				vr_renderWindowInteractor[k]->TerminateApp(); // stop if the v-rep simulation is not running
			}
			return;
		}
	}
}

void stereoPanorama_renderwindow_support::activateMainCam(int height) {
	int goodRender;
	simxGetIntegerSignal(clientID, "High_quality_render", &goodRender, simx_opmode_blocking); // whatever this is

	simxFloat *data;
	simxInt dataLength;
	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"getEstetics"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, simx_opmode_blocking);

	for (int k = 0; k < vr_camera.size(); k++) {
		vr_camera[k]->SetViewAngle(90.0);
		vr_camera[k]->SetPosition(0, 0, 0);
		vr_camera[k]->SetFocalPoint(0, 0, 1);
		vr_camera[k]->SetViewUp(0, 1, 0);
		vr_camera[k]->SetModelTransformMatrix(pose[k]->GetMatrix());
		// Classical vtk pipeline to set up renderwindow etc with extra options
		renderer[k]->SetActiveCamera(vr_camera[k]);
		if (goodRender == 1) {
			renderer[k]->UseShadowsOn();
		}
		renderer[k]->SetTwoSidedLighting(false);
		renderer[k]->LightFollowCameraOff();
		renderer[k]->AutomaticLightCreationOff();
		renderer[k]->UseDepthPeelingForVolumesOn();
		renderer[k]->SetUseDepthPeeling(true);
		renderer[k]->SetMaximumNumberOfPeels(20);
		renderer[k]->Modified();
		//renderer[k]->SetBackground(data[0], data[1], data[2]);
		//renderer[k]->SetBackground2(data[3], data[4], data[5]);
		//renderer[k]->SetGradientBackground(true);
		//renderer[k]->SetBackground(1,1,1);
		renderer[k]->SetBackgroundAlpha(0.0);  //-> re activate this
		//renderer[k]->SetUseFXAA(true);
		
		renderWindow[k]->AddRenderer(renderer[k]);

		renderWindow[k]->SetSize(121.0, height); // make shure we have a 'middle' pixel
		renderWindow[k]->SetOffScreenRendering(true);
		renderWindow[k]->Initialize();
		renderWindow[k]->SetDesiredUpdateRate(10000.0);
		renderWindow[k]->SetAlphaBitPlanes(1); // -> re activate this
		//renderWindow[k]->SetAlphaBitPlanes(true);
		renderWindow[k]->SetMultiSamples(0);
		vr_renderWindowInteractor[k]->SetRenderWindow(renderWindow[k]);
		vr_renderWindowInteractor[k]->Initialize();
		filter[k]->SetInput(renderWindow[k]);
		filter[k]->SetInputBufferTypeToRGBA(); //-> re activate this
		filter[k]->ShouldRerenderOff();
	}
}



