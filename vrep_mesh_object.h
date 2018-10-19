// Copyright (c) 2017, Boris Bogaerts
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

#include "vtkTransform.h"
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkActor.h>
#include <string>
#include "vrep_mesh_reader.h"
#include <vtkProperty.h>
#include "extApi.h"
#include <vector>
#include <vtkPNGReader.h>
#include <vtkTexture.h>
#include <vtkPolyData.h>
#include <vtkOBJReader.h>
#include <vtkPointData.h>

#ifndef vrep_mesh_object_H
#define vrep_mesh_object_H
class vrep_mesh_object
{
public:
	vrep_mesh_object();
	vrep_mesh_object(const vrep_mesh_object& other);
	~vrep_mesh_object();

	void setColor(float color[3]);
	void setOpacity(float opacity) { vrep_mesh_opacity = opacity; };
	void setHandle(int handle) { vrep_mesh_handle = handle; };
	void setMeshData(vtkSmartPointer<vtkPolyData> mesh) { meshData = mesh; };
	void setName(std::string name) { std::move(vrep_model_name = name); };
	std::string getName() { return std::move(vrep_model_name); };
	void setClientID(int ID, int refH) { clientID = ID; refHandle = refH; };
	void extractDataFromReader(vrep_mesh_reader reader);
	void makeActor();
	vtkSmartPointer<vtkActor> getActor() { return vrep_mesh_actor; };
	void updatePosition();
	vtkSmartPointer<vtkPolyData> getMeshData() { return meshData; };
	vtkSmartPointer<vtkTransform> getPose() { return pose; };
	// copy functions for setting cam objects
	int getHandle() { return vrep_mesh_handle; };
	void getHandles();
	vtkSmartPointer<vtkTransform> getCamTransform();
	void setActor(vtkSmartPointer<vtkActor> act);
	void deepCopy(vrep_mesh_object *newObject);
protected:
	vtkSmartPointer<vtkTransform> pose = vtkSmartPointer<vtkTransform>::New();
	vtkSmartPointer<vtkPolyData> meshData = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyDataMapper> vrep_polyData_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> vrep_mesh_actor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();

	float vrep_mesh_color[3];
	float vrep_mesh_opacity;
	int vrep_mesh_handle;
	int clientID;
	int refHandle;
	bool texturedObject = false;
	int updateCounter = 0;
	std::string vrep_model_name;
	// texture related things
	
};

#endif