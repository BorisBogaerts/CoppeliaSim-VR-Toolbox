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

#include "vrep_volume_grid.h"
#include <vtkFloatArray.h>
#include <vtkCellData.h>

#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkDataObject.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <ppl.h>
//#include <LuaBridge.h>
using namespace std;
vrep_volume_grid::~vrep_volume_grid()
{
	pose->Delete();
	vertices->Delete();
	grid->Delete();
	volume->Delete();
	mapper->Delete();
	scalar->Delete();
}

bool vrep_volume_grid::updatePosition(vtkSmartPointer<vtkFloatArray> values) {
	if (alternativeHandle == -1) {
		bool update = this->updateGrid();
		if (update) { return true; };
		simxFloat eulerAngles[3];
		simxFloat position[3];
		simxGetObjectOrientation(clientID, objectHandle, refHandle, eulerAngles, simx_opmode_streaming); // later replace by : simx_opmode_buffer 
		simxGetObjectPosition(clientID, objectHandle, refHandle, position, simx_opmode_streaming);
		pose->PostMultiply();
		pose->Identity();

		pose->RotateZ((eulerAngles[2] * 180 / 3.1415));
		pose->RotateY((eulerAngles[1] * 180 / 3.1415));
		pose->RotateX((eulerAngles[0] * 180 / 3.1415));

		pose->Translate(position);

		pose->RotateX(-90);
		pose->Modified();
	}
	scalar->ShallowCopy(values);
	//scalar->DeepCopy(values);
	//scalar = values;
	//scalar->Modified();
	return false;
}

bool vrep_volume_grid::updateGrid() {
	simxFloat *data;
	simxInt dataLength;
	simxCallScriptFunction(clientID, (simxChar*)"Field", sim_scripttype_childscript, (simxChar*)"getUpdate"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	if (dataLength > 0) {
		this->loadGrid();
		grid->Modified();
		return true;
	}
	return false;
}

bool vrep_volume_grid::loadGrid() {
	simxFloat *data;
	simxInt dataLength;
	simxInt *intData;
	simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"helloVolume"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &dataLength, &intData, NULL, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	if (intData[0] == -1) {
		return false;
	}
	simxCallScriptFunction(clientID, (simxChar*)"Field", sim_scripttype_childscript, (simxChar*)"getData"
		, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, simx_opmode_blocking);

	if (dataLength == 1) {
		alternativeHandle = (int)data[0];
		return false;
	}
	grid->SetOrigin(data[4], data[5], data[6]);
	grid->SetSpacing(data[3], data[3], data[3]);	grid->SetDimensions(data[0], data[1], data[2]);
	objectHandle = data[7];
	vertices->SetDataTypeToFloat();
	scalar->SetNumberOfComponents(1);
	scalar->SetNumberOfValues(grid->GetNumberOfPoints());

	vertices->SetNumberOfPoints(grid->GetNumberOfPoints());
	for (int i = 1; i < grid->GetNumberOfPoints(); i++) {
		scalar->SetValue(i, (float)rand() / (RAND_MAX));
		vertices->InsertPoint(i, grid->GetPoint(i));
	}
	scalar->SetName("scalars");
	grid->GetPointData()->SetScalars(scalar);
	grid->GetPointData()->SetActiveScalars("scalars");

	// Flush this function (important)
	simxInt *lengths;
	simxInt numLengths;
	for (int i = 1; i < 10; i++) {
		simxCallScriptFunction(clientID, (simxChar*)"Field", sim_scripttype_childscript, (simxChar*)"getColorMapData"
			, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &numLengths, &lengths, &dataLength, &data, NULL, NULL, NULL, NULL, simx_opmode_streaming);
	}
	return true;
}

void vrep_volume_grid::updatMap() {
	setColorMap(currentMode);
}

void vrep_volume_grid::setColorMap(int mode) {
	vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	if (mode == 1) {
		// Get colormap data from vrep
		simxFloat *data;
		simxInt dataLength;
		simxInt *lengths;
		simxInt numLengths;

		simxCallScriptFunction(clientID, (simxChar*)"Field", sim_scripttype_childscript, (simxChar*)"getColorMapData"
			, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &numLengths, &lengths, &dataLength, &data, NULL, NULL, NULL, NULL, simx_opmode_streaming);
		for (int i = 0; i < lengths[0]; i++) {
			opacityTransferFunction->AddPoint(data[i * 2] / 4.0, data[(i * 2) + 1]);

		}
		//cout << endl;
		for (int i = 0; i < lengths[1]; i++) {
			int beginID = (lengths[0] * 2) + (i * 4);
			colorTransferFunction->AddRGBPoint(data[beginID] / 4.0, data[beginID + 1], data[beginID + 2], data[beginID + 3]);
			//cout << "Point added : " << data[beginID] << "	" << data[beginID + 1] << "	" << data[beginID + 2] << "	" << data[beginID + 3] << endl;
		}
		//opacityTransferFunction->AddPoint(0, 0.0);
		//opacityTransferFunction->AddPoint(1, 0.1);
		//opacityTransferFunction->AddPoint(2, 0.1);
		//opacityTransferFunction->AddPoint(3, 0.2);
	/*	colorTransferFunction->AddRGBPoint(0, 0, 0, 1);
		colorTransferFunction->AddRGBPoint(0.25, 0, 0.5, 0.5);
		colorTransferFunction->AddRGBPoint(0.5, 0, 0.8, 0.0);
		colorTransferFunction->AddRGBPoint(0.75, 0.5, 0.5, 0.8);
		colorTransferFunction->AddRGBPoint(1.0, 0.8, 0, 0);*/
	}
	else {
		opacityTransferFunction->AddPoint(0.0, 0.7);
		opacityTransferFunction->AddPoint(0.125, 0);
		opacityTransferFunction->AddPoint(0.25, 0);
		colorTransferFunction->AddRGBPoint(0.0, 1.0, 0.0, 0.0);
	}
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetColor(colorTransferFunction);
	volumeProperty->SetScalarOpacity(opacityTransferFunction);
	volumeProperty->SetScalarOpacityUnitDistance(1);
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->ShadeOff();

	volume->SetProperty(volumeProperty);
	volume->Modified();
	currentMode = mode;
}

void vrep_volume_grid::toggleMode() {
	if (currentMode == 1) {
		setColorMap(2);
	}
	else {
		setColorMap(1);
	}
}

vtkSmartPointer<vtkVolume> vrep_volume_grid::getVolume() {
	mapper->SetInputData(grid);
	mapper->AutoAdjustSampleDistancesOff();
	mapper->SetScalarModeToUsePointData();
	mapper->SelectScalarArray("scalars");
	//mapper->SetRequestedRenderModeToRayCast();
	//mapper->SetRequestedRenderModeToGPU();
	volume->SetMapper(mapper);
	mapper->SetLockSampleDistanceToInputSpacing(true);
	mapper->UseJitteringOn();
	mapper->SetBlendModeToComposite();
	setColorMap(2);
	volume->SetUserTransform(pose);

	volume->PickableOff();
	return volume;
}

vtkSmartPointer<vtkVolume> vrep_volume_grid::getNewVolume() {
	vtkSmartPointer<vtkVolume> newVolume = vtkSmartPointer<vtkVolume>::New();
	vtkSmartPointer<vtkImageData> newGrid = vtkSmartPointer<vtkImageData>::New();
	vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> newMapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
	newGrid->ShallowCopy(grid);
	newMapper->SetInputData(grid);
	newMapper->AutoAdjustSampleDistancesOff();
	newMapper->SetScalarModeToUsePointData();
	newMapper->SelectScalarArray("scalars");
	newMapper->SetLockSampleDistanceToInputSpacing(true);
	newMapper->UseJitteringOn();
	newMapper->SetBlendModeToComposite();
	newVolume->SetMapper(newMapper);
	newVolume->SetProperty(volume->GetProperty());
	newVolume->SetUserTransform(pose);
	newVolume->PickableOff();
	return newVolume;
}

vtkSmartPointer<vtkLookupTable> vrep_volume_grid::getLUT(int numValues) {
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorTransferFunction->AddRGBPoint(0, 1.0, 1.0, 1.0); // start with white

	// cool to warm colormap, should work better: https://www.kennethmoreland.com/color-maps/ (not for me at least)
	//colorTransferFunction->AddRGBPoint(0.01, 0.706, 0.016, 0.150); 
	//colorTransferFunction->AddRGBPoint(1, 0.230, 0.299, 0.754);

	// MATLAB inverted parula
	colorTransferFunction->AddRGBPoint(0.01, 0.9769, 0.9839, 0.0805);
	colorTransferFunction->AddRGBPoint(0.05, 0.0704, 0.7457, 0.7258);
	colorTransferFunction->AddRGBPoint(1, 0.2422, 0.1504, 0.6603);

	LUT->SetNumberOfTableValues(numValues); // Build lookup table
	LUT->SetRange(0, 1);

	for (int i = 0; i < numValues; ++i) {
		double *rgb;
		rgb = colorTransferFunction->GetColor(static_cast<double>(i) / numValues);
		rgb[3] = 1.0;
		//std::cout << rgb[0] << "   " << rgb[1] << "   " << rgb[2] << endl;
		LUT->SetTableValue(i, rgb);
	}
	LUT->Build();
	return LUT;
}