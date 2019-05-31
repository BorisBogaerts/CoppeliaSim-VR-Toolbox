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

#include "pathObject.h"
#include <iostream>
#include <cstdlib>
#include <vtkLinearTransform.h>
#include <vtkIdList.h>
pathObject::pathObject(int ID)
{
	//return;
	clientID = ID;

	simxInt *data;
	simxInt dataLength;
	simxInt result;
	//result = simxCallScriptFunction(clientID, (simxChar*)"Camera_feeder", sim_scripttype_childscript, (simxChar*)"helloPath"
	//	, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);
	//if (result == 0) {
		//if (data[0] == 1) {
			cout << "Connected to VREP path" << endl;
			exist = true;
			simxInt succes;
			simxUChar *str;
			simxInt signalLength;
			succes = simxGetStringSignal(clientID, (simxChar*)"path", &str, &signalLength, simx_opmode_streaming); // start stream

		//}
	//}
}


pathObject::~pathObject()
{
}

vtkSmartPointer<vtkActor> pathObject::getActor() {
	update();
	cells->InsertNextCell(lineData);
	points->Allocate(20000);
	cells->Allocate(20000);
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
		succes = simxGetStringSignal(clientID, (simxChar*)"path", &str, &signalLength, simx_opmode_streaming);
 		if (succes == simx_return_ok) {
			signalLength = (int)(signalLength / 4);
			if (signalLength > prevLength) {
				points->SetNumberOfPoints((int)(signalLength / 3));
				memcpy(points->GetVoidPointer(0), (float*)str, signalLength * sizeof(float));
				//cells->SetNumberOfCells((int)((signalLength / 3)));
				//cells->Initialize();
				for (int i = cells->GetNumberOfCells() + 1; i < (int)((signalLength / 3)); i++) {
					cells->InsertNextCell(2);
					cells->InsertCellPoint(i - 1);
					cells->InsertCellPoint(i);
				}
				points->Modified();
				cells->Modified();
				polyData->Modified();
				prevLength = signalLength;
			}else if(signalLength < prevLength){
				points->SetNumberOfPoints((int)(signalLength / 3)); //set all points anew
				cells->Reset();
				for (int i = 1; i < (int)((signalLength / 3)); i++) {
					cells->InsertNextCell(2);
					cells->InsertCellPoint(i - 1);
					cells->InsertCellPoint(i);
				}
				points->Modified();
				cells->Modified();
				polyData->Modified();
				prevLength = signalLength;
			}
			else {
				memcpy(points->GetVoidPointer(0), (float*)str, signalLength * sizeof(float)); // for if somebody else changed the path, some optimizer maybe?
				points->Modified();
				polyData->Modified();
			}
		}
		return;
	}
}