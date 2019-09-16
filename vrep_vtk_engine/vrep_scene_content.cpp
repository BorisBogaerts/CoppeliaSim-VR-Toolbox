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
// OF T
#pragma once
#include <chrono>
#include "vrep_scene_content.h"
#include "vrep_mesh_reader.h"
#include <vector>
#include "extApi.h"
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>

#include "vrep_controlled_object.h"
#include <vtkOpenGLRenderer.h>
#include <vtkFloatArray.h>
#include <thread>
#include <ppl.h>
// vrep_scene_content //

vrep_scene_content::~vrep_scene_content() {
}


void vrep_scene_content::loadScene(bool doubleScene) {
	int counter = 0;
	int v = 10;
	int h = 0;
	int clientID = 0;
	// interpret extra instructions
	vrepMeshContainer.reserve(2000); // I do not know why, but this is necessary

	if (doubleScene) { 
		vrepMeshContainer2.reserve(2000);
	};

	// Get all the object handles that need to be loaded
	int *handles;
	int numHandles;
	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"getVisibleHandles"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &numHandles, &handles, NULL, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	// read geometry
	int goodRender;
	float dump;
	simxGetIntegerSignal(clientID, "High_quality_render", &goodRender, simx_opmode_streaming); // whatever this is
	simxGetFloatSignal(clientID, "AmbientStrength", &dump, simx_opmode_streaming);
	simxGetFloatSignal(clientID, "DiffuseStrength", &dump, simx_opmode_streaming);
	simxGetFloatSignal(clientID, "SpecularStrength", &dump, simx_opmode_streaming);
	simxGetFloatSignal(clientID, "SpecularPower", &dump, simx_opmode_streaming);
	int* han = new int[numHandles];
	for (int i = 0; i < numHandles; i++) {
		han[i] = handles[i];
	}
	for (int i = 0; i < numHandles; i++) {
		vrep_mesh_reader temp;
		vrep_mesh_object temp2;
		temp.read_mesh(clientID, han[i], h, v);
		if (v > 0) {
			// Get data for vr 				
			vrepMeshContainer.push_back(temp2);
			vrepMeshContainer[vrepMeshContainer.size()-1].extractDataFromReader(temp);
			// General things
			vrepMeshContainer[vrepMeshContainer.size() - 1].setClientID(clientID, refHandle);
			vrepMeshContainer[vrepMeshContainer.size() - 1].setHandle(h);
			if (doubleScene) {
				vrep_mesh_object temp3;
				vrepMeshContainer2.push_back(temp3);
				vrepMeshContainer[vrepMeshContainer.size() - 1].deepCopy(&vrepMeshContainer2[vrepMeshContainer.size() - 1]); // copy temp2 data in temp3
			}
			vrepMeshContainer[vrepMeshContainer.size() - 1].makeActor();
		}
	}
	cout << "Number of objects loaded : " << vrepMeshContainer.size() << endl;		

	// read lights
	simxInt *dataInt;
	simxFloat *dataFloat;
	simxInt dataFloatLength;
	simxInt dataIntLength;
	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"getLightInfo"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &dataIntLength, &dataInt, &dataFloatLength, &dataFloat, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	for (int i = 0; i < dataInt[0]; i++) {
		vrep_light newl(dataInt[i+1], refHandle, clientID);
		lights.push_back(newl);
		float color1[3] = { dataFloat[(i*11)] ,dataFloat[(i*11) + 1] ,dataFloat[(i*11) + 2] };
		float color2[3] = { dataFloat[(i * 11)+3] ,dataFloat[(i * 11) + 4] ,dataFloat[(i * 11) + 5] };
		float angle = dataFloat[(i * 11) + 6];
		float exponent = dataFloat[(i * 11) + 7];
		float constAtten = dataFloat[(i * 11) + 8];
		float linAtten = dataFloat[(i * 11) + 9];
		float quadAtten = dataFloat[(i * 11) + 10];
		lights.back().createLight(color1, color2, angle, exponent,constAtten,linAtten,quadAtten);
		cout << "Added light, angle : " << angle << endl;
	}
}

void vrep_scene_content::dynamicLoad() {
	int v = 10;
	int h = 0;
	int clientID = 0;

	// Get all the object handles that need to be loaded
	int *handles;
	int numHandles;
	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"getVisibleHandlesDynamic"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &numHandles, &handles, NULL, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	// read geometry
	int* han = new int[numHandles];
	for (int i = 0; i < numHandles; i++) {
		han[i] = handles[i];
	}
	for (int i = 0; i < numHandles; i++) {
		vrep_mesh_reader temp;
		vrep_mesh_object temp2;
		temp.read_mesh(clientID, han[i], h, v);
		if (v > 0) {
			// Get data for vr 				
			vrepMeshContainer.push_back(temp2);
			vrepMeshContainer[vrepMeshContainer.size() - 1].extractDataFromReader(temp);
			// General things
			vrepMeshContainer[vrepMeshContainer.size() - 1].setClientID(clientID, refHandle);
			vrepMeshContainer[vrepMeshContainer.size() - 1].setHandle(h);
			vrepMeshContainer[vrepMeshContainer.size() - 1].makeActor();
		}
	}
}

void vrep_scene_content::activateNewConnection() {
	int clientID2 = simxStart((simxChar*)"127.0.0.1", 19998, true, true, 10000, 5);
	cout << "Connection opened on port 19998 (second thread)" << endl;
	for (int i = 0; i < vrepMeshContainer2.size(); i++) {
		vrepMeshContainer2[i].setClientID(clientID2, refHandle); // second core uses a new connection
	}
}

void vrep_scene_content::loadVolume() {
	// load volume
	vol->setClientID(clientID, refHandle);
	bool succes = vol->loadGrid();
	if (succes) {
		volume = vol->getVolume();
		//volume->SetEstimatedRenderTime(0);
		volumePresent = true;
	}
}

void vrep_scene_content::loadCams() {
	camsContainer.reserve(20);
	simxFloat *data;
	simxInt dataLength;
	simxInt *intData;
	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"helloCams"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &dataLength, &intData, NULL, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);

	if (intData[0] == -1) {
		return;
	}
	simxCallScriptFunction(clientID, (simxChar*)"Camera_feeder", sim_scripttype_childscript, (simxChar*)"getData"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	if (dataLength > 0) {
		std::vector<float> canvasHandles;
		std::vector<float> handleHandles;
		for (int i = 2; i < dataLength; i = i + 10) {
			canvasHandles.push_back(data[i]); // list all canvases
		}
		for (int i = 1; i < dataLength; i = i + 10) {
			handleHandles.push_back(data[i]); // list all handles
		}

		for (int i = 0; i < dataLength; i = i + 10) {
			vrep_vision_sensor temp = vrep_vision_sensor();
			temp.setClientID(clientID, refHandle);
			camsContainer.push_back(temp);
			float cameraParameters[] = { data[i+3], data[i + 4], data[i + 5], data[i + 6], data[i + 7] };
			camsContainer[camsContainer.size() - 1].setCameraParams(data[i], cameraParameters);
			int panelIndex = 0;
			// now load all actors
			for (int k = 0; k < this->getNumActors(); k++) {
				bool add = true;
				bool setPanel = false;
				for (const int& ii : canvasHandles) {
					if (ii==vrepMeshContainer[k].getHandle()) { 
						if (ii == data[i + 2]) {
							setPanel = true; 
						}
						add = false;
					}
				}
				for (const int& ii : handleHandles) {
					if (ii == vrepMeshContainer[k].getHandle()) {
						add = false;
						vrepMeshContainer[k].getActor()->PickableOff();
						vrepMeshContainer[k].getActor()->Modified();
					}
				}
				if (add) {
					vtkSmartPointer<vtkActor> tempAct = vrepMeshContainer2[k].getActor();
					tempAct->PickableOff();
					tempAct->GetProperty()->SetOpacity(1.0);
					camsContainer[camsContainer.size() - 1].getRenderer()->AddActor(tempAct); // do not add this to the renders
				}
				if (setPanel) {
					this->getActor(k)->PickableOff();
					camsContainer[camsContainer.size() - 1].setPanel(this->getActor(k)); panelIndex = k;
					camsContainer[camsContainer.size() - 1].setPose2(vrepMeshContainer[k].getPose());
				}
			}
			double dim[] = { data[i + 8] , data[i + 9] };
			camsContainer[camsContainer.size() - 1].activate(dim); // standard vtk pipeline
			vrepMeshContainer[panelIndex].setActor(camsContainer[camsContainer.size() - 1].getActor());
		}
	}
	else {
		cout << "No vision sensors found" << endl;
	};
}



vtkSmartPointer<vtkActor> vrep_scene_content::getActor(int i) {
	return vrepMeshContainer[i].getActor();
}

vtkSmartPointer<vtkActor> vrep_scene_content::getNewActor(int i) {
	return vrepMeshContainer[i].getNewActor();
}

vtkSmartPointer<vtkActor> vrep_scene_content::getActor2(int i) {
	return vrepMeshContainer2[i].getActor();
}


int vrep_scene_content::getNumActors() {
	return vrepMeshContainer.size();
}


void vrep_scene_content::updateMainCamObjectPose() {
	for (int i = 0; i < vrepMeshContainer.size(); i++) {
		vrepMeshContainer[i].updatePosition();
	}
	for (int i = 0; i < camsContainer.size(); i++) {
		camsContainer[i].updatePosition();
	}
	for (int i = 0; i < lights.size(); i++) {
		lights[i].updatePose();
	}
}

void vrep_scene_content::updateVisionSensorObjectPose() {
	for (int i = 0; i < vrepMeshContainer2.size(); i++) {
		vrepMeshContainer2[i].updatePosition();
	}
}

float vrep_scene_content::updateVisionSensorRender() {
	float ret = 0;
//#pragma loop(hint_parallel(4))  
	for (int i = 0; i < camsContainer.size(); i++) {
		camsContainer[i].updateRender();
	}
	ret = this->computeScalarField();
	return ret;
}

void vrep_scene_content::transferVisionSensorData(vtkSmartPointer<vtkOpenVRRenderWindowInteractor> iren, vtkSmartPointer<vtkOpenVRRenderWindow> win, vtkSmartPointer<vtkOpenVRRenderer> ren) {
//pragma loop(hint_parallel(4))  
	for (int i = 0; i < camsContainer.size(); i++) {
		camsContainer[i].transferImageTexture();
	}
	iren->DoOneEvent(win, ren);
	bool update = vol->updatePosition(scalar);
}

void vrep_scene_content::transferVisionSensorData() {
#pragma loop(hint_parallel(6))  
	for (int i = 0; i < camsContainer.size(); i++) {
		if (i < camsContainer.size()) {
			camsContainer[i].transferImageTexture();
		}
		
	}
	if (isVolumePresent()){
		bool update = vol->updatePosition(scalar);
	}
	
}

void vrep_scene_content::vrep_get_object_pose() {
	updateMainCamObjectPose();
	if (isVolumePresent()) {
		updateVisionSensorObjectPose();
		updateVisionSensorRender();
		transferVisionSensorData();
	}
}

float vrep_scene_content::computeScalarField() {
	if (firstTime) {
		if (volumePresent) {
			//scalar = vol->getScalars();
			scalar->SetNumberOfValues(vol->getNumberOfValues());
			state->SetNumberOfValues(vol->getNumberOfValues());
		}
		else {
			state->SetNumberOfValues(numPoints);
			scalar->SetNumberOfValues(numPoints);
		}
		scalar->FillValue((float)0);
		state->FillValue((float)0);
		firstTime = false;
	}
	
	
	int reset, inProgress=0;
	if (!integrateMeasurement) {
		scalar->FillValue((float)0); // if not integration just clear previous
		count = 0;
	}else {
		simxGetIntegerSignal(clientID, "ResetMeasurement", &reset, simx_opmode_streaming); // ask V-REP if we need to clear the measurementState
		simxGetIntegerSignal(clientID, "MeasurementInProgress", &inProgress, simx_opmode_streaming); // ask vrep if measurement is in progress
		if (reset > 0) {
			simxSetIntegerSignal(clientID, "ResetMeasurement", 0, simx_opmode_blocking); // reset is received and acted upon
			state->FillValue((float)0); // reset history
		}
		scalar->DeepCopy(state); // needed to allow a "preview" without having to "measure", thus field is adapted but not stored if not measuring
	}


		for (int i = 0; i <camsContainer.size(); i++) {
			vtkSmartPointer<vtkPolyData> temp = camsContainer[i].checkVisibility();
			for (int ii = 0; ii < temp->GetPointData()->GetArray(0)->GetNumberOfTuples(); ii++) {
				int ID = temp->GetPointData()->GetArray(0)->GetTuple1(ii); // vertex ID (array only contains visible poins, other points are ommited)
				float prevVal = scalar->GetValue(ID);  // if integration is not active then 
				float newqual;
				if (camsContainer[i].useQuality()) {
					newqual = std::max(prevVal, (float)temp->GetPointData()->GetArray(1)->GetTuple1(ii) / qualityThreshold);
				}else {
					newqual = std::min((float)1.0, prevVal + 1);
				}
				scalar->SetValue(ID, newqual); // if quality value is available, use this (clip quality)
				if ((prevVal < newqual) && (newqual==1.0)) {
					count++;
				}
			}
		}
		if (inProgress > 0) {
			state->DeepCopy(scalar);
		}
		scalar->Modified();
		float coverage = (float)count / scalar->GetNumberOfValues();
		return coverage;
}

void vrep_scene_content::connectCamsToVolume() {
	if (volumePresent) {
		for (int i = 0; i < camsContainer.size(); i++) {
			camsContainer[i].setPointData(vol->getPoints(), vol->getTransform());
			activeThread = true;
		}
	}
}

void vrep_scene_content::checkMeasurementObject() {
	if (volumePresent) { // volume is occupied by volume
		return;
	}
	for (int j = 0; j < vrepMeshContainer2.size(); j++) {
		vrepMeshContainer2[j].getActor()->GetProperty()->SetColor(0.0, 0.0, 0.0);
	}
	if (vol->getAltHandle() == -1) { // no alternative handle was returned so no child
		return;
	}
	// An alternative mesh is provided, we should fix some stuff
	cout << "OK switching from volume to inspection mesh with handle : " << vol->getAltHandle() << endl;
	for (int i = 0; i < vrepMeshContainer.size(); i++) {
		if (vol->getAltHandle() == vrepMeshContainer[i].getHandle()) { // This is the one
			float color[] = { 1.0,1.0,1.0 }; // base color white
			vrepMeshContainer[i].setColor(color);
			//vtkSmartPointer<vtkFloatArray> scalar = vol->getScalars(); // get the scalar field (contains quality per vertex)
			vtkSmartPointer<vtkPolyData> meshData = vrepMeshContainer[i].getMeshData(); // get the mesh
			meshH = i;
			// now itialize scalars
			scalar->SetNumberOfComponents(1);
			scalar->SetNumberOfValues(meshData->GetNumberOfPoints()); // one value for each vertex
			numPoints = meshData->GetNumberOfPoints();
			scalar->FillValue(0.0); 
			scalar->SetName("scalars");
			meshData->GetPointData()->SetScalars(scalar); // add scalars to the mesh
			meshData->GetPointData()->SetActiveScalars("scalars"); // make them "active"

			// stupid change of data object switch
			vtkSmartPointer<vtkPoints> vertices = vtkSmartPointer<vtkPoints>::New();
			for (int ii = 0; ii < meshData->GetNumberOfPoints(); ii++) {
				vertices->InsertPoint(ii, meshData->GetPoint(ii));
			}
			// Use quality or not?
			int *sizes;
			int numVals;
			simxFloat *data;
			simxInt dataLength;
			simxCallScriptFunction(clientID, (simxChar*)"Camera_feeder", sim_scripttype_childscript, (simxChar*)"getQualityMode"
				, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &numVals, &sizes, &dataLength, &data,NULL, NULL, NULL, NULL, simx_opmode_blocking);
			bool useCustomQuality;
			integrateMeasurement = (sizes[1] > 0);
			if (integrateMeasurement) {
				for (int j = 0; j < 10; j++) { // flush
					int reset, inProgress;
					simxGetIntegerSignal(clientID, "ResetMeasurement", &reset, simx_opmode_streaming); // ask V-REP if we need to clear the measurementState
					simxGetIntegerSignal(clientID, "MeasurementInProgress", &inProgress, simx_opmode_streaming); // ask vrep if measurement is in progress
				}
			}
			if (sizes[0] > 0) {
				useCustomQuality = true;
				qualityThreshold = std::max((float)data[0], (float)1e-3); // otherwise we might divide by zero
				vrepMeshContainer2[i].setCustomShader(); // compute custom measurement quality function
			}
			else {
				useCustomQuality = false;
			}
			// Now connect the points to the vision sensor (these mesh points will be used in the visibility check)
			for (int ii = 0; ii < camsContainer.size(); ii++) {
				camsContainer[ii].setPointData(vertices, vrepMeshContainer[i].getPose());
				camsContainer[ii].setQualityMode(useCustomQuality);
				activeThread = true;
			}

			vrepMeshContainer[i].getMapper()->SetLookupTable(vol->getLUT(100)); // sets colormap  
			//volumePresent = true;
		}
	}
}
