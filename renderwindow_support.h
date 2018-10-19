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