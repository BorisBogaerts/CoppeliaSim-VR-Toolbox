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

#include "vrep_mesh_reader.h"
#include "extApi.h"
#include <iostream>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vector>
#include <vtkPointData.h>

int vrep_mesh_reader::read_mesh(int clientID, int meshIndex, int &h, int &v) {
	int *sizes;
	int numVals;
	simxFloat *data = new simxFloat[500000];
	simxInt dataLength;
	char* name;
	simxInt nameLength;
	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"getGeometryInformation"
		, 1, &meshIndex, 0, NULL, 0, NULL, 0, NULL, &numVals,  &sizes, &dataLength, &data, &nameLength, &name, NULL, NULL, simx_opmode_blocking);

	for (int i = 0; i < nameLength; i++) {
		objectName.append(&name[i]);
	};

	if (sizes[0] > 0) {
		std::cout << "Loaded geometry with handle " << meshIndex << " with " << sizes[0]/3 << " vertices and " << sizes[1]/3 << " faces" << " vrep name : " << objectName << " Visibility layer : " << sizes[5] << std::endl;
	}
	

	// Now put the data in the right format 
	vtkSmartPointer<vtkPoints> vertices = vtkSmartPointer<vtkPoints>::New(); // define vertices container
	vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New(); // define face container
	vertices->SetDataTypeToFloat();
	vertices->SetNumberOfPoints(sizes[0] / 3);
	faces->SetNumberOfCells(sizes[1] / 3);
	// Get vertices fromta
	int baseIndex;
	for (int i = 0; i < sizes[0] / 3; i++) {
		baseIndex = i * 3;
		vertices->InsertPoint(i, data[baseIndex], data[baseIndex + 1], data[baseIndex + 2]);
	}

	vtkIdType temp[3];
	// Get faces from data
	for (int i = 0; i < sizes[1] / 3; i++) {
		baseIndex = sizes[0]+ (i * 3);
		for (int j = 0; j < 3; j++) {
			temp[j] = data[baseIndex + j];// -1;
		}	
		faces->InsertNextCell(3, temp);
	}
	// Get color from data
	
	int counter = 0;
	for (int i = 0; i<3; i++) {
		baseIndex = sizes[0] + sizes[1] + i;
		color[counter] = data[baseIndex];
		counter++;
	}
	opacity = data[sizes[0] + sizes[1] + 3];
	// create polydata (mesh) object
	mesh->SetPoints(vertices);
	mesh->SetPolys(faces);
	v = sizes[0] / 3;
	h = sizes[3];
	if (sizes[4] == 1) {
		this->texture = true;
	}
	visibilityLayer = sizes[5];
	return sizes[2];
}

vtkSmartPointer<vtkPolyData> vrep_mesh_reader::getMeshData() {
	return mesh;
}

