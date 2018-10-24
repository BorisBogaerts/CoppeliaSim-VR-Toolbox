#include "pathObject.h"
#include <iostream>
#include <cstdlib>
#include <vtkLinearTransform.h>
#include <vtkIdList.h>
pathObject::pathObject(int ID)
{
	clientID = ID;
	simxInt succes;
	simxUChar *str;
	simxInt signalLength;
		succes = simxGetStringSignal(clientID, (simxChar*)"path", &str, &signalLength, simx_opmode_streaming);
		if (succes == simx_return_ok) { // if signal exist than we will are in business, otherwise ignore
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
		if (succes == simx_return_ok) {
			signalLength = (int)(signalLength / 4);
			points->SetNumberOfPoints((int)(signalLength / 3));
			float* convert = (float*)str;
			memcpy(points->GetVoidPointer(0), convert, signalLength * sizeof(float));

			for (int i = 0; i < points->GetNumberOfPoints(); i++) {
				cout << points->GetPoint(i)[0] << "     " << points->GetPoint(i)[1] << "     " << points->GetPoint(i)[2] << endl;
			}
			//cells->SetNumberOfCells((int)((signalLength / 3)));
			//cells->Initialize();
			for (int i = cells->GetNumberOfCells()+1; i < (int)((signalLength / 3)); i++) {
			//for (int i = 1; i < (int)((signalLength / 3)); i++) {
				vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
					cells->InsertNextCell(2);
					cells->InsertCellPoint(i-1);
					cells->InsertCellPoint(i);
			}
			////cells->GetData()->Modified();
			//cout << cells->GetNumberOfCells() << endl;
			//cells->InitTraversal();
			//for (int i = 0; i < cells->GetNumberOfCells(); i++) {
			//	vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
			//	cells->GetCell(i, ids);
			//	cout << ids->GetNumberOfIds() << endl;
			//	cout << ids->GetId(0) << "    " << ids->GetId(1) << endl;
			//}
			points->Modified();
			cells->Modified();
		}
		return;
	}
}