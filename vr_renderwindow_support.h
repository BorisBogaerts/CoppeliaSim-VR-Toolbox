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
	vr_renderwindow_support(int cid);
	vr_renderwindow_support() { chrono->coverage = &coverage; chrono->scale = &scale;};
	~vr_renderwindow_support();

	void addVrepScene(vrep_scene_content *vrepSceneIn);
	void activate_interactor();
	void updatePose();
	void updateRender();
	void setClientID(int cid) { clientID = cid; };
	vrep_scene_content * getVrepScene() { return vrepScene; };
	
	void visionSensorThread();
	bool isReady() { return dataReady; };
	void setNotReady() { dataReady = false; };

	
	void syncData(vtkSmartPointer<vtkOpenVRRenderWindowInteractor> iren, vtkSmartPointer<vtkOpenVRRenderWindow> win, vtkSmartPointer<vtkOpenVRRenderer> ren);
	void discoverDevices();
	void synchronizeDevices();

	void updateText();
	timerClass *chrono = new timerClass;
protected:
	vrep_scene_content *vrepScene;
	int clientID;
	int handle;
	int update = 10;

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

