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
#include <vtkWin32RenderWindowInteractor.h>
#include <vtkOpenGLRenderer.h>
#include <vtkWin32OpenGLRenderWindow.h>
#include <vtkOpenGLCamera.h>
#include "vrep_scene_content.h"
#include "vrep_volume_grid.h"
#include <vtkTransform.h>
#include <vtkTextureUnitManager.h>
#include <vtkWindowToImageFilter.h>
#include <vtkVolume.h>
#include <vtkImageAppend.h>

#include "vrep_volume_grid.h"
#include "timerClass.h"
#include "vrep_controlled_object.h"
#include "pathObject.h"
#include <vector>
#include <vtkExtractVOI.h>
#include <bitset>
#include <atomic>

#pragma once
class stereoPanorama_renderwindow_support
{
public:
	stereoPanorama_renderwindow_support(int cid, int ref, int interactor);
	~stereoPanorama_renderwindow_support();

	void addVrepScene(vrep_scene_content *vrepSceneIn);
	void activate_interactor();
	void updatePose();
	void setClientID(int cid, int interactor) { clientID = cid; useInteractor = (interactor == 0); };
	vrep_scene_content * getVrepScene() { return vrepScene; };
	void renderStrip(float dist, bool left, bool top, int k, int width, int height);
	bool isReady() { return dataReady; };
	void setNotReady() { dataReady = false; };

	void activateMainCam(int height);
	void checkLayers();
	void dynamicAddObjects();

	void syncData();
	//void checkBackground();
	timerClass *chrono = new timerClass;
protected:
	vrep_scene_content * vrepScene;
	int clientID;
	int handle;
	int lastObject = 0;

	int refH;
	int update = 10;
	int useInteractor = true;
	std::vector<vtkSmartPointer<vtkRenderer>> renderer;
	std::vector<vtkSmartPointer<vtkWin32OpenGLRenderWindow>> renderWindow;
	std::vector<vtkSmartPointer<vtkWin32RenderWindowInteractor>> vr_renderWindowInteractor;
	std::vector<vtkSmartPointer<vtkOpenGLCamera>> vr_camera;
	std::vector<vtkSmartPointer<vtkTransform>> pose;
	std::vector<vtkSmartPointer<vtkImageData>> slice;

	//vtkSmartPointer<vtkExtractVOI> backGroundSlicer = nullptr;
	//vtkSmartPointer<vtkImageData> backGround = nullptr;

	std::vector<vtkSmartPointer<vtkWindowToImageFilter>> filter;

	vrep_volume_grid *grid;
	pathObject *path;
	std::atomic<bool> busy = true;
	std::atomic<bool> locked = false;
	std::vector<std::bitset<16>> visibilityLayer;
	float coverage = 0;
	bool dataReady = false;
	int textUpdateCounter = 0;
};
