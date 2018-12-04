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

#include "vrep_mesh_object.h"
#include <vtkPlaneSource.h>
#include <vtkDataArray.h>
#include <vtkLandmarkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkCellArray.h>
#include <vtkProperty.h>
#include <string>
#include <vtkPNGReader.h>
#include <vtkFloatArray.h>
#include <cmath>
#include <dense>
#include <vtkMatrix4x4.h>

#define PI 3.14f

vrep_mesh_object::vrep_mesh_object() {
}

vrep_mesh_object::~vrep_mesh_object(){

}

vrep_mesh_object::vrep_mesh_object(const vrep_mesh_object& other){
}

void vrep_mesh_object::setColor(float color[3]) {
	for (int i = 0; i < 3; i++) {
		vrep_mesh_color[i] = color[i];
	}
};

void vrep_mesh_object::extractDataFromReader(vrep_mesh_reader reader) {
	this->setMeshData(reader.getMeshData());
	this->setName(reader.getName());
	this->setColor(reader.color);
	this->setOpacity(reader.opacity);
	this->texturedObject = reader.getTexture();
	vrep_mesh_actor->PickableOff();
};

void vrep_mesh_object::makeActor() {
	vrep_polyData_mapper->SetInputData(meshData);
	vrep_mesh_actor->SetMapper(vrep_polyData_mapper);

	if (texturedObject) {
			std::string textureName;
			simxFloat *data;
			simxInt dataLength;
			char* name;
			simxInt nameLength;
			simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"getTextureInformation"
				, 1, &vrep_mesh_handle, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, &dataLength, &data, &nameLength, &name, NULL, NULL, simx_opmode_blocking); 
			for (int i = 0; i < nameLength; i++) {
				textureName.append(&name[i]);
			};
			vtkSmartPointer<vtkPNGReader> io = vtkSmartPointer<vtkPNGReader>::New();
			io->SetFileName(textureName.c_str());
			texture->SetInputConnection(io->GetOutputPort());
			//texture->MipmapOn();
			texture->InterpolateOn();
			texture->Update();
			vtkSmartPointer<vtkFloatArray> tccoords = vtkSmartPointer<vtkFloatArray>::New();
			tccoords->SetNumberOfComponents(2);
			for (int i = 0; i < (dataLength/ 2) ; i++) {
				tccoords->InsertNextTuple2(data[2 * i], data[2 * i + 1]);
			}
			meshData->GetPointData()->SetTCoords(tccoords);
			meshData->Modified();
		vrep_mesh_actor->SetTexture(texture);
		cout << "    Added texture to shape" << endl;
	}
	else {
		vrep_mesh_actor->GetProperty()->SetColor(vrep_mesh_color[0], vrep_mesh_color[1], vrep_mesh_color[2]);
		vrep_mesh_actor->GetProperty()->SetOpacity(vrep_mesh_opacity);
	};
	vrep_mesh_actor->SetUserTransform(pose);
};

void vrep_mesh_object::setActor(vtkSmartPointer<vtkActor> act) {
	vrep_mesh_actor = act;
	vrep_mesh_actor->SetUserTransform(pose);
}

void vrep_mesh_object::updatePosition() {
	//if ((vrep_mesh_actor->GetPickable() == 0) || (updateCounter<10)){ // vrep position updates vtk position
		simxFloat eulerAngles[3];
		simxFloat position[3];

		simxGetObjectOrientation(clientID, vrep_mesh_handle, refHandle, eulerAngles, simx_opmode_streaming); // later replace by : simx_opmode_buffer 
		simxGetObjectPosition(clientID, vrep_mesh_handle, refHandle, position, simx_opmode_streaming);
		pose->PostMultiply();
		pose->Identity();

		pose->RotateZ((eulerAngles[2] * 180 / 3.1415));
		pose->RotateY((eulerAngles[1] * 180 / 3.1415));
		pose->RotateX((eulerAngles[0] * 180 / 3.1415));

		pose->Translate(position);

		pose->RotateX(-90);
		if (updateCounter < 11) { updateCounter++; };
	//}
	//else { // vtk position updates vtk position
	//	cout << "Something wrong" << endl;
	//	using ::Eigen::AngleAxisd;
	//	using ::Eigen::Matrix3d;
	//	using ::Eigen::Vector3d;
	//	double orr[4];
	//	float pos[3];
	//	vtkSmartPointer<vtkTransform> Tt = vtkSmartPointer<vtkTransform>::New();
	//	vrep_mesh_actor->GlobalWarningDisplayOff();
	//	Tt->GetMatrix()->DeepCopy(vrep_mesh_actor->GetUserTransform()->GetMatrix());
	//	Tt->PostMultiply();
	//	Tt->RotateX(90);
	//	Tt->GetPosition(pos);
	//	//Tt->Inverse();
	//	//Tt->GetOrientation(orr);
	//	vtkSmartPointer<vtkTransform> textra = vtkSmartPointer<vtkTransform>::New();
	//	textra->Identity();
	//	textra->PostMultiply();
	//	//textra->RotateX(-180);
	//	//textra->RotateZ(-180);
	//	textra->Inverse();

	//	textra->Concatenate(Tt);
	//	//Tt->Concatenate(textra);
	//	textra->GetOrientationWXYZ(orr);
	//	double orrt[3];
	//	for (int i = 1; i < 4; i++) {
	//		orrt[i - 1] = orr[i];
	//	}
	//	Matrix3d R;
	//	AngleAxisd angleAxis(PI* orr[0] / 180, Vector3d(orrt));
	//	R = AngleAxisd(angleAxis);
	//	Vector3d ea = R.eulerAngles(0, 1, 2);
	//	float temp[3];
	//	for (int i = 0; i < 3; i++) {
	//		temp[i] = ea[i];
	//	}
	//	simxSetObjectOrientation(clientID, vrep_mesh_handle, refHandle, temp, simx_opmode_streaming);
	//	simxSetObjectPosition(clientID, vrep_mesh_handle, refHandle, pos, simx_opmode_streaming);
	//}
	
};

void vrep_mesh_object::getHandles() {
	simxGetObjectHandle(clientID, vrep_model_name.c_str(), &vrep_mesh_handle, simx_opmode_blocking);
};

vtkSmartPointer<vtkTransform> vrep_mesh_object::getCamTransform() {
	vtkSmartPointer<vtkTransform> T = vtkSmartPointer<vtkTransform>::New();
	simxFloat eulerAngles[3];
	simxFloat position[3];

	simxGetObjectOrientation(clientID, vrep_mesh_handle, -1, eulerAngles, simx_opmode_streaming); // later replace by : simx_opmode_buffer 
	simxGetObjectPosition(clientID, vrep_mesh_handle, -1, position, simx_opmode_streaming);
	T->PostMultiply();
	T->Identity();

	T->RotateZ((eulerAngles[2] * 180 / 3.1415));
	T->RotateY((eulerAngles[1] * 180 / 3.1415));
	T->RotateX((eulerAngles[0] * 180 / 3.1415));

	T->Translate(position);

	T->RotateX(-90);
	vtkSmartPointer<vtkTransform> textra = vtkSmartPointer<vtkTransform>::New();
	textra->Identity();
	textra->PostMultiply();
	textra->RotateX(-180);
	textra->RotateZ(-180);
	textra->Inverse();

	textra->Concatenate(T);
	return textra;
}

void vrep_mesh_object::deepCopy(vrep_mesh_object *newObject) {
	newObject->meshData->DeepCopy(meshData);
	newObject->setColor(vrep_mesh_color);
	newObject->setOpacity(vrep_mesh_opacity);
	newObject->setHandle(vrep_mesh_handle);
	newObject->setName(vrep_model_name);
	newObject->setClientID(clientID, refHandle);
	newObject->makeActor();
}
