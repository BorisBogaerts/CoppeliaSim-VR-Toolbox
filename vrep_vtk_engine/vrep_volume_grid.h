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

#include "extApi.h"
#include <iostream>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <iostream>
#include <vtkVolume.h>
#include <vtkPointSet.h>
#include <vtkImageData.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkFloatArray.h>
#include <vtkTransform.h>
#include <vtkLookupTable.h>
#pragma once
class vrep_volume_grid
{
public:
	vrep_volume_grid() {  };
	void setClientID(int cid, int rh) { clientID = cid; refHandle = rh; };
	~vrep_volume_grid();
	bool loadGrid();
	void updatMap();
	vtkSmartPointer<vtkVolume> getVolume();
	vtkSmartPointer<vtkVolume> getNewVolume();
	bool updatePosition(vtkSmartPointer<vtkFloatArray> values);
	bool updateGrid();
	vtkSmartPointer<vtkImageData> getGrid() { return grid; };
	vtkSmartPointer<vtkPoints> getPoints() { return vertices; };
	vtkSmartPointer<vtkTransform> getTransform() { return pose; ; }
	int getNumberOfValues() { return grid->GetNumberOfPoints(); };
	void setColorMap(int mode);
	void toggleMode();
	int getAltHandle() { return alternativeHandle; };
	vtkSmartPointer<vtkLookupTable> getLUT(int numValues);
	vtkSmartPointer<vtkFloatArray> getScalars() { return scalar; };
protected:
	int clientID;
	int refHandle;
	int objectHandle;
	int currentMode = 0;
	int alternativeHandle = -1;

	vtkSmartPointer<vtkTransform> pose = vtkSmartPointer<vtkTransform>::New();
	vtkSmartPointer<vtkPoints> vertices = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkImageData> grid = vtkSmartPointer<vtkImageData>::New();
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> mapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
	vtkSmartPointer<vtkFloatArray> scalar = vtkSmartPointer<vtkFloatArray>::New();
	vtkSmartPointer<vtkLookupTable> LUT = vtkSmartPointer<vtkLookupTable>::New();
};

