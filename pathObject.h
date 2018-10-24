#pragma once
#include "extApi.h"
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkProperty.h>
#include <vtkPolyLine.h>
#include <vtkTransform.h>
class pathObject
{
public:
	pathObject(int ID);
	~pathObject();
	void update();
	vtkSmartPointer<vtkActor> getActor();
protected:
	int clientID;
	bool exist = false;

	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkCellArray> cells =vtkSmartPointer<vtkCellArray>::New();
	
	vtkSmartPointer<vtkPolyLine> lineData = vtkSmartPointer<vtkPolyLine>::New();
	vtkSmartPointer<vtkPolyDataMapper> lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> lineActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkTransform> pose = vtkSmartPointer<vtkTransform>::New();

};

