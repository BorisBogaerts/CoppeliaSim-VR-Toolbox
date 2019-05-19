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
#include <vtkTransform.h>
#include <vtkWindowToImageFilter.h>
#include <vtkImageData.h>
#include <vtkActor.h>
#include <vtkOpenGLTexture.h>
#include <vtkPoints.h>

#include <vtkIdFilter.h>
#include <vtkTransformFilter.h>
#include <vtkSelectVisiblePoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>

#include <vtkPlaneSource.h>
#include <vtkTextureMapToPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkCameraActor.h>
#include <vtkImageResize.h>
class vrep_vision_sensor
{
public:
	vrep_vision_sensor() {};
	~vrep_vision_sensor() {};
	void updateRender();
	void setCameraParams(int h, float cameraParams[]);
	void updatePosition();
	void setClientID(int cid, int rh) { clientID = cid; refH = rh; };
	void activate(double scale[]);
	void activateBasic();
	void setPanel(vtkSmartPointer<vtkActor> pnl) { panel = pnl; };
	vtkSmartPointer<vtkOpenGLRenderer> getRenderer() { return renderer; };
	vtkSmartPointer<vtkActor> getActor() { return panel; };
	vtkSmartPointer<vtkActor> getNewactor();
	vtkSmartPointer<vtkPolyData> checkVisibility();
	void setPose2(vtkSmartPointer<vtkTransform> temp) { pose2 = temp; };
	void setPointData(vtkSmartPointer<vtkPoints> data, vtkSmartPointer<vtkTransform> pose);
	void updatePose();
	void transferImageTexture();
	vtkSmartPointer<vtkPolyData> getQuality();
	bool useQuality() { return computeQuality; };
	void setQualityMode(bool mode) { computeQuality = mode; }
private:
	int clientID;
	int handle;
	int update = 10;
	bool basic = false;
	int refH;
	bool computeQuality = false;
	float scaleX, scaleY;

	vtkSmartPointer<vtkOpenGLRenderer> renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
	vtkSmartPointer<vtkWin32OpenGLRenderWindow> renderWindow = vtkSmartPointer<vtkWin32OpenGLRenderWindow>::New();
	vtkSmartPointer<vtkWin32RenderWindowInteractor> vr_renderWindowInteractor = vtkSmartPointer<vtkWin32RenderWindowInteractor>::New();
	vtkSmartPointer<vtkOpenGLCamera> vr_camera = vtkSmartPointer<vtkOpenGLCamera>::New();
	vtkSmartPointer<vtkTransform> invPose = vtkSmartPointer<vtkTransform>::New();
	vtkSmartPointer<vtkWindowToImageFilter> filter = vtkSmartPointer<vtkWindowToImageFilter>::New();
	vtkSmartPointer<vtkActor> panel = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkOpenGLTexture> texture = vtkSmartPointer<vtkOpenGLTexture>::New();
	vtkSmartPointer<vtkOpenGLTexture> texture2 = vtkSmartPointer<vtkOpenGLTexture>::New();
	vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
	std::vector<vtkSmartPointer<vtkImageData>> extraImages;
	std::vector<vtkSmartPointer<vtkTexture>> extraTextures;
	vtkSmartPointer<vtkTransform> pose2;

	vtkSmartPointer<vtkTransformFilter > ptsT = vtkSmartPointer<vtkTransformFilter >::New();
	vtkSmartPointer<vtkSelectVisiblePoints> selectVisiblePoints = vtkSmartPointer<vtkSelectVisiblePoints>::New();
	vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkPolyData> ptset = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkIdFilter> ids = vtkSmartPointer<vtkIdFilter>::New();
	vtkSmartPointer<vtkTransform> pose = vtkSmartPointer<vtkTransform>::New();
};

