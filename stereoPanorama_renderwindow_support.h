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

