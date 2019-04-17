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

#include <vtkSmartPointer.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRCamera.h>
#include "vrep_scene_content.h"
#include "vrep_volume_grid.h"

#include "vrep_volume_grid.h"
#include <vtkVolume.h>
#include <vtkOpenVRInteractorStyle.h>
#include "timerClass.h"
#include "vrep_controlled_object.h"
#include "pathObject.h"
#include <vtkVectorText.h>
#include <vtkLinearExtrusionFilter.h>
#pragma once
class vr_renderwindow_support
{
public:
	vr_renderwindow_support(int cid, int ref, int interactor);
	vr_renderwindow_support() { chrono->coverage = &coverage; chrono->scale = &scale;};
	~vr_renderwindow_support();

	void addVrepScene(vrep_scene_content *vrepSceneIn);
	void activate_interactor();
	void updatePose();
	void updateRender();
	void setClientID(int cid, int interactor) { clientID = cid; useInteractor = (interactor == 0); };
	vrep_scene_content * getVrepScene() { return vrepScene; };
	void dynamicAddObjects();
	void visionSensorThread();
	bool isReady() { return dataReady; };
	void setNotReady() { dataReady = false; };

	
	void syncData();
	void discoverDevices();
	void synchronizeDevices();


	void updateText();
	timerClass *chrono = new timerClass;
protected:
	vrep_scene_content *vrepScene;
	int clientID;
	int handle;
	
	int refH;
	int update = 10;
	int useInteractor = true;
	int lastObject = 0;
	vtkSmartPointer<vtkOpenVRRenderer> renderer = vtkSmartPointer<vtkOpenVRRenderer>::New();
	vtkSmartPointer<vtkOpenVRRenderWindow> renderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
	vtkSmartPointer<vtkOpenVRRenderWindowInteractor> vr_renderWindowInteractor = vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New();
	vtkSmartPointer<vtkOpenVRCamera> vr_camera = vtkSmartPointer<vtkOpenVRCamera>::New();
	vtkSmartPointer<vtkOpenVRInteractorStyle> style = vtkSmartPointer<vtkOpenVRInteractorStyle>::New();

	vrep_controlled_object *controller1_vrep;
	vrep_controlled_object *controller2_vrep;
	vrep_controlled_object *headset_vrep;

	vrep_volume_grid *grid;
	vrep_vision_sensor *screenCam;
	pathObject *path;

	vtkSmartPointer<vtkVectorText> vecText = vtkSmartPointer<vtkVectorText>::New();
	vtkSmartPointer<vtkLinearExtrusionFilter> extrude = vtkSmartPointer<vtkLinearExtrusionFilter>::New();
	vtkSmartPointer<vtkPolyDataMapper> txtMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> txtActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkTransform> Tt = vtkSmartPointer<vtkTransform>::New();

	bool screenCamInUse = false;
	float coverage = 0;
	float scale = 1;
	bool dataReady = false;
	int textUpdateCounter = 0;
	int vtkDevices[3] = { -1, -1,-1 };
};

