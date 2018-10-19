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
	bool updatePosition(vtkSmartPointer<vtkFloatArray> values);
	bool updateGrid();
	vtkSmartPointer<vtkImageData> getGrid() { return grid; };
	vtkSmartPointer<vtkPoints> getPoints() { return vertices; };
	vtkSmartPointer<vtkTransform> getTransform() { return pose; ; }
	int getNumberOfValues() { return grid->GetNumberOfPoints(); };
	void setColorMap(int mode);
	void toggleMode();
	int getAltHandle() { return alternativeHandle; };
protected:
	int clientID;
	int refHandle;
	int objectHandle;
	int currentMode = 0;
	int alternativeHandle = 0;

	vtkSmartPointer<vtkTransform> pose = vtkSmartPointer<vtkTransform>::New();
	vtkSmartPointer<vtkPoints> vertices = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkImageData> grid = vtkSmartPointer<vtkImageData>::New();
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> mapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
	vtkSmartPointer<vtkFloatArray> scalar = vtkSmartPointer<vtkFloatArray>::New();
};

