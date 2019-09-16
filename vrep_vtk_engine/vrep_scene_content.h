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
#include "vrep_mesh_reader.h"
#include <vtkTransform.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

#include <vtkWindowToImageFilter.h>
#include "vrep_mesh_object.h"
#include <vector>
#include <vtkLight.h>
#include <vtkActor.h>
#include "vrep_volume_grid.h"
#include <vtkVolume.h>
#include "vrep_vision_sensor.h"
#include <vtkPoints.h>
#include <vtkCameraActor.h>
#include "vrep_light.h"
#include <vtkLightActor.h>

#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRCamera.h>

//#include <LuaBridge.h>
//extern "C" {
//# include "lua.h"
//# include "lauxlib.h"
//# include "lualib.h"
//}
#ifndef vrep_scene_content_H
#define vrep_scene_content_H

class vrep_scene_content
{
public:
	vrep_scene_content(int cid, int refH) { clientID = cid; refHandle = refH; };
	~vrep_scene_content();

	void loadScene(bool doubleScene);
	void loadVolume();
	void vrep_get_object_pose();
	std::vector<vrep_mesh_object> getVrepSceneActors() { return vrepMeshContainer; };
	vtkSmartPointer<vtkActor> getActor(int i);
	vtkSmartPointer<vtkActor> getActor2(int i);
	int getNumActors();
	int getNumRenders() { return camsContainer.size(); };
	vtkSmartPointer<vtkVolume> getVolume() { return volume; };
	vtkSmartPointer<vtkVolume> getNawVolume() { return vol->getNewVolume(); }
	vtkSmartPointer<vtkActor> getPanelActor(int i) { return camsContainer[i].getActor(); };
	vtkSmartPointer<vtkActor> getNewPanelActor(int i) { return camsContainer[i].getNewactor(); };
	vtkSmartPointer<vtkLight> getLight(int i) { return lights[i].getLight(); };
	vtkSmartPointer<vtkLightActor> getLightActor(int i) { return lights[i].getActor(); };
	vtkSmartPointer<vtkLight>getNewLight(int i) { return lights[i].getNewLight(); }
	int getNumberOfLights() { return lights.size(); };
	float computeScalarField();
	bool isVolumePresent() { return volumePresent; };
	void loadCams();
	void checkMeasurementObject();
	void connectCamsToVolume();
	void activateNewConnection();
	void updateMainCamObjectPose();
	void updateVisionSensorObjectPose();
	float updateVisionSensorRender();
	void transferVisionSensorData(vtkSmartPointer<vtkOpenVRRenderWindowInteractor> iren, vtkSmartPointer<vtkOpenVRRenderWindow> win, vtkSmartPointer<vtkOpenVRRenderer> ren);
	void transferVisionSensorData();
	bool startVisionSensorThread() { return activeThread; };
	vrep_volume_grid *vol = new vrep_volume_grid();
	void dynamicLoad();
	int getVisibilityLayer(int num) { return vrepMeshContainer[num].visibilityLayer; };
	vtkSmartPointer<vtkActor> getNewActor(int i);
protected:
	int clientID;
	int refHandle;
	bool volumePresent = false;
	bool activeThread = false;
	int numPoints;
	float qualityThreshold = 4.0;
	bool integrateMeasurement = false;
	int count = 0;
	bool firstTime = true;
	vtkSmartPointer<vtkVolume> volume;
	

	std::vector<vrep_vision_sensor> camsContainer;
	std::vector<vrep_mesh_object> vrepMeshContainer;
	std::vector<vrep_mesh_object> vrepMeshContainer2;
	std::vector<vrep_light> lights;
	int meshH;

	vtkSmartPointer<vtkFloatArray> scalar = vtkSmartPointer<vtkFloatArray>::New();
	vtkSmartPointer<vtkFloatArray> state = vtkSmartPointer<vtkFloatArray>::New();
};

#endif