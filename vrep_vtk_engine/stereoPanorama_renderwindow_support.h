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
#include "vr_renderwindow_support.h"
#include <vtkSmartPointer.h>
#include "vrep_scene_content.h"
#include "vrep_volume_grid.h"

#include "vrep_volume_grid.h"
#include <vtkVolume.h>
#include "timerClass.h"
#include "vrep_controlled_object.h"
#include <vtkWin32OpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkWin32RenderWindowInteractor.h>
#include <vtkOpenGLCamera.h>

class stereoPanorama_renderwindow_support
{
public:
	stereoPanorama_renderwindow_support(int cid) { clientID = cid; chrono->coverage = &coverage; chrono->scale = &scale; };
	~stereoPanorama_renderwindow_support();

	void updateRender();
	void addVrepScene(vrep_scene_content *vrepSceneIn);
	void updatePose();
	void syncData(vtkSmartPointer<vtkWin32OpenGLRenderWindow> win);
	void visionSensorThread();
	void activate_interactor();

	bool isReady() { return dataReady; };
	void setNotReady() { dataReady = false; };

	void Initialize();

	timerClass *chrono = new timerClass;
protected:
	vrep_scene_content * vrepScene;
	int clientID;
	int handle;
	int update = 10;
	int counter = 0;

	vtkSmartPointer<vtkWin32OpenGLRenderWindow> renderWindow = vtkSmartPointer<vtkWin32OpenGLRenderWindow>::New();

	vrep_volume_grid *grid;

	float coverage = 0;
	float scale = 1;
	bool dataReady = false;
};

