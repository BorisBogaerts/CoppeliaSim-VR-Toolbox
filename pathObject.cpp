#include "pathObject.h"
#include <iostream>
#include <cstdlib>
#include <vtkLinearTransform.h>
#include <vtkIdList.h>
pathObject::pathObject(int ID)
{
	//return;
	clientID = ID;
	simxInt succes;
	simxUChar *str;
	simxInt signalLength;
		succes = simxGetStringSignal(clientID, (simxChar*)"path", &str, &signalLength, simx_opmode_streaming);
		if (succes <2) { // if signal exist than we will are in business, otherwise ignore
			cout << "Connected to VREP path" << endl;
			exist = true;
		}
}


pathObject::~pathObject()
{
}

vtkSmartPointer<vtkActor> pathObject::getActor() {
	update();
	cells->InsertNextCell(lineData);

	polyData->SetPoints(points);
	polyData->SetLines(cells);

	lineMapper->SetInputData(polyData);
	lineActor->SetMapper(lineMapper);
	lineActor->GetProperty()->SetColor(0.7, 0.14, 0.56);
	lineActor->GetProperty()->SetLineWidth(5.0);
	lineActor->PickableOff();
	pose->Identity();
	pose->RotateX(-90);
	lineActor->SetUserTransform(pose);
	return lineActor;
}

void pathObject::update() {
	if (exist) {
		simxInt succes;
		simxUChar *str;
		simxInt signalLength;
		succes = simxGetStringSignal(clientID, (simxChar*)"path", &str, &signalLength, simx_opmode_buffer);
		//cout << "Path status : " << succes << endl;
		
		if (succes == simx_return_ok) {
			signalLength = (int)(signalLength / 4);
			points->SetNumberOfPoints((int)(signalLength / 3));
			memcpy(points->GetVoidPointer(0), (float*)str, signalLength * sizeof(float));
			//cells->SetNumberOfCells((int)((signalLength / 3)));
			//cells->Initialize();
			for (int i = cells->GetNumberOfCells()+1; i < (int)((signalLength / 3)); i++) {
					cells->InsertNextCell(2);
					cells->InsertCellPoint(i-1);
					cells->InsertCellPoint(i);
			}
		
		
			points->Modified();
			cells->Modified();
			polyData->Modified();
		}
		return;
	}
}

//for (int i = 0; i < points->GetNumberOfPoints(); i++) {
//	cout << points->GetPoint(i)[0] << "     " << points->GetPoint(i)[1] << "     " << points->GetPoint(i)[2] << endl;
//}

//for (int i = 0; i < cells->GetNumberOfCells(); i++) {
//	vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
//	cells->GetCell(i, ids);
//	cout << ids->GetNumberOfIds() << endl;
//	cout << ids->GetId(0) << "    " << ids->GetId(1) << endl;
//}