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

#include <vtkSmartPointer.h>
#include <vtkWin32OpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkWin32RenderWindowInteractor.h>
#include <vtkOpenGLCamera.h>
#include "vrep_scene_content.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vrep_volume_grid.h"
#include <vtkVolume.h>
#include "timerClass.h"


#ifndef renderwindow_support_H
#define renderwindow_support_H

class renderwindow_support
{
public:
	renderwindow_support(int cid) { this->setClientID(cid); };
	renderwindow_support() {};
	~renderwindow_support();
	void addVrepScene(vrep_scene_content *vrepSceneIn);
	void activate_interactor();
	void updatePose();
	void updateRender();
	void setClientID(int cid) { clientID = cid; };
	void visionSensorThread();
	bool isReady() { return dataReady; };
	void setNotReady() { dataReady = false; };

	void syncData();

	int countRender = 0;
	int countVision = 0;

	timerClass *chrono = new timerClass;
private:
	vrep_scene_content *vrepScene;
	int clientID;
	int handle;
	int update = 10;

	vtkSmartPointer<vtkOpenGLRenderer> renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
	vtkSmartPointer<vtkWin32OpenGLRenderWindow> renderWindow = vtkSmartPointer<vtkWin32OpenGLRenderWindow>::New();
	vtkSmartPointer<vtkWin32RenderWindowInteractor> vr_renderWindowInteractor = vtkSmartPointer<vtkWin32RenderWindowInteractor>::New();
	vtkSmartPointer<vtkOpenGLCamera> vr_camera = vtkSmartPointer<vtkOpenGLCamera>::New();

	bool dataReady = false;
};

#endif