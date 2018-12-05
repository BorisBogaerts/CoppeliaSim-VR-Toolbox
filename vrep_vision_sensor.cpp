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

#include "vrep_vision_sensor.h"
#include <vector>
#include "extApi.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>
void vrep_vision_sensor::updatePosition() {
	this->updatePose();
	invPose->DeepCopy(pose);
	invPose->Inverse();
	vr_camera->SetModelTransformMatrix(pose->GetMatrix());
	vr_camera->Modified();
}

void vrep_vision_sensor::updatePose() {
	simxFloat eulerAngles[3];
	simxFloat position[3];

	simxGetObjectOrientation(clientID, handle, refH, eulerAngles, simx_opmode_streaming); // later replace by : simx_opmode_buffer 
	simxGetObjectPosition(clientID, handle, refH, position, simx_opmode_streaming);
	pose->PostMultiply();
	pose->Identity();
	pose->RotateZ((eulerAngles[2] * 180 / 3.1415));
	pose->RotateY((eulerAngles[1] * 180 / 3.1415));
	pose->RotateX((eulerAngles[0] * 180 / 3.1415));

	pose->Translate(position);
	pose->RotateX(-90);

	pose->Inverse();
	pose->Modified();
}

vtkSmartPointer<vtkPolyData> vrep_vision_sensor::checkVisibility() {
	ptsT->Update();
	selectVisiblePoints->Update();
	return selectVisiblePoints->GetOutput();
}

void vrep_vision_sensor::setPointData(vtkSmartPointer<vtkPoints> data, vtkSmartPointer<vtkTransform> pose) {
	data->SetDataTypeToFloat();
	pts->DeepCopy(data); // we do not want any trouble
	ptset->SetPoints(pts);
	vtkSmartPointer<vtkIntArray> id = vtkSmartPointer<vtkIntArray>::New();
	id->SetNumberOfValues(ptset->GetNumberOfPoints());
	for (int i = 0; i < ptset->GetNumberOfPoints(); i++) {
		id->SetValue(i, i);
	}
	ptset->GetPointData()->AddArray(id);
	ptsT->SetInputData(ptset);
	ptsT->SetTransform(pose);

	selectVisiblePoints->SetTolerance(1e-4);
	selectVisiblePoints->SelectionWindowOff();
	selectVisiblePoints->SetInputConnection(ptsT->GetOutputPort());
	selectVisiblePoints->SetRenderer(renderer);
	selectVisiblePoints->Update();
}


void vrep_vision_sensor::setCameraParams(int h, float cameraParams[]) {
	handle = h;
	vr_camera->SetViewAngle(180*cameraParams[2]/3.1415); // also change winsow size
	renderWindow->SetSize(cameraParams[0], cameraParams[1]);
	double range[] = { cameraParams[3], cameraParams[4] };
	vr_camera->SetClippingRange(range);
	vr_camera->SetModelTransformMatrix(pose->GetMatrix());
	vr_camera->SetPosition(0, 0, 0);
	vr_camera->SetFocalPoint(0, 0, 1);
	vr_camera->SetViewUp(0, -1, 0);
	//vr_camera->SetUserTransform(pose);
	//vr_camera->SetUserViewTransform(pose);
	cout << "Vision sensor : px" << cameraParams[0] << "x" << cameraParams[1] << "| persp angle " << 180 * cameraParams[2] / 3.1415<< "| Clipping planes " << cameraParams[3] << "-" << cameraParams[4] << endl;
}

void vrep_vision_sensor::updateRender() {
	if (basic) {
		updatePosition();
		renderWindow->Render();
	}
	else {
		renderWindow->Render();
		//texture->Render(renderer);
		filter->Modified();
		filter->ReadFrontBufferOff();
		filter->Update();
		//resize->SetOutputDimensions(100,100,1);
		//resize->Update();
		//image->ShallowCopy(filter->GetOutput());
	}
};

void vrep_vision_sensor::transferImageTexture() {
	//image->Modified();
	image->ShallowCopy(filter->GetOutput());
	image->Modified();
}


void vrep_vision_sensor::activate(double scale[]) {
	// add actors
	// maybe hide renderwindow

	vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
	vtkSmartPointer<vtkTextureMapToPlane> texturePlane = vtkSmartPointer<vtkTextureMapToPlane>::New();
	vtkSmartPointer<vtkPolyDataMapper> planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	vtkSmartPointer<vtkTransformFilter> transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
	
	transform->Scale(scale[0], scale[1], 1);
	transform->RotateZ(180);
	transform->RotateY(180);

	texturePlane->SetInputConnection(plane->GetOutputPort());
	transformFilter->SetInputConnection(texturePlane->GetOutputPort());

	transformFilter->SetTransform(transform);
	planeMapper->SetInputConnection(transformFilter->GetOutputPort());

	panel->SetMapper(planeMapper);
	renderer->SetBackground(1.0, 1.0, 1.0);
	renderer->SetActiveCamera(vr_camera);
	
	renderWindow->AddRenderer(renderer);
	vr_renderWindowInteractor->SetRenderWindow(renderWindow);
	vr_renderWindowInteractor->Render();
	vr_renderWindowInteractor->Disable();
	renderWindow->OffScreenRenderingOn();
	renderWindow->SetDesiredUpdateRate(500.0);
	filter->SetInput(renderWindow);
	//resize->SetInputConnection(filter->GetOutputPort()); // extra
	//texture->SetInputData(image);
	texture->SetInputConnection(filter->GetOutputPort());
	texture2->SetInputData(image);
	panel->SetTexture(texture2);
	panel->Modified();

}

void vrep_vision_sensor::activateBasic() {
	basic = true;
	renderer->SetActiveCamera(vr_camera);
	renderer->SetBackground(1.0, 1.0, 1.0);
	renderWindow->AddRenderer(renderer);
	vr_renderWindowInteractor->SetRenderWindow(renderWindow);
	renderWindow->SetDesiredUpdateRate(200.0);
	//vr_renderWindowInteractor->SetDesiredUpdateRate(200.0);
	//vr_renderWindowInteractor->Initialize();
	renderWindow->Render();
	//vr_renderWindowInteractor->Render();
}