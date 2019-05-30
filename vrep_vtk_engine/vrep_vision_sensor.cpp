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
#include <vtkMatrix4x4.h>
#include <vtkFloatArray.h>
#include <vtkProperty.h>

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
	if (computeQuality) {
		return getQuality();
	}
	return selectVisiblePoints->GetOutput();
}

vtkSmartPointer<vtkPolyData> vrep_vision_sensor::getQuality() {
	vtkSmartPointer<vtkPolyData> data = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkFloatArray> quality = vtkSmartPointer<vtkFloatArray>::New();
	data = selectVisiblePoints->GetOutput();
	quality->SetNumberOfValues(data->GetNumberOfPoints());
	double x[3];
	double view[4];
	double dx[3];
	double xx[4] = { x[0], x[1], x[2], 1.0 };
	float pix;
	double * displayCoordinates;
	vtkSmartPointer<vtkMatrix4x4> M = vtkSmartPointer<vtkMatrix4x4>::New();
	M = vr_camera->GetCompositeProjectionTransformMatrix(renderer->GetTiledAspectRatio(), 0, 1); // get the transorm
	int dims[3];
	filter->GetOutput()->GetDimensions(dims);
	// Transformation sequence is discussed in : https://public.kitware.com/pipermail/vtkusers/2010-September/062478.html
	for (int i = 0; i < data->GetNumberOfPoints(); i++) {
		data->GetPoint(i, x); // get a mesh point
		renderer->SetWorldPoint(x);
		renderer->WorldToView();
		displayCoordinates = renderer->GetViewPoint();
		renderer->ViewToDisplay();
		renderer->GetDisplayPoint(dx);
		// Following check is necessary because a slight change is position is posible
		if ((static_cast<int>(dx[0]) >= 0) && (static_cast<int>(dx[0]) < dims[0]) && (static_cast<int>(dx[1]) >= 0) && (static_cast<int>(dx[1]) < dims[1])) {
			pix = filter->GetOutput()->GetScalarComponentAsFloat(static_cast<int>(dx[0]), static_cast<int>(dx[1]), 0, 2) / 255; // rendered image is also in the texture so we'll get it there because it is easier)
			quality->SetValue(i, pix);
		}
		else {
			quality->SetValue(i, 0.0);
		}
	}
	data->GetPointData()->AddArray(quality);
	return data;
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

	selectVisiblePoints->SetTolerance(5e-4);
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
		renderer->Render();
		filter->Modified();
		filter->ReadFrontBufferOff();
		filter->Update();
	}
};

void vrep_vision_sensor::transferImageTexture() {
	image->ShallowCopy(filter->GetOutput());
	image->Modified();
	for (int i = 0; i < extraImages.size(); i++) {
		extraImages[i]->DeepCopy(filter->GetOutput());
		extraImages[i]->Modified();
	}
}


void vrep_vision_sensor::activate(double scale[]) {
	// add actors
	// maybe hide renderwindow

	vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
	vtkSmartPointer<vtkTextureMapToPlane> texturePlane = vtkSmartPointer<vtkTextureMapToPlane>::New();
	vtkSmartPointer<vtkPolyDataMapper> planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	vtkSmartPointer<vtkTransformFilter> transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
	scaleX = scale[0];
	scaleY = scale[1];
	transform->Scale(scale[0], scale[1], 1);
	transform->RotateZ(180);
	transform->RotateY(180);

	texturePlane->SetInputConnection(plane->GetOutputPort());
	transformFilter->SetInputConnection(texturePlane->GetOutputPort());

	transformFilter->SetTransform(transform);
	planeMapper->SetInputConnection(transformFilter->GetOutputPort());
	planeMapper->Update();

	panel->SetMapper(planeMapper);
	renderer->SetBackground(0.0, 0.0, 0.0);
	renderer->SetActiveCamera(vr_camera);
	
	renderWindow->AddRenderer(renderer);
	vr_renderWindowInteractor->SetRenderWindow(renderWindow);
	vr_renderWindowInteractor->Render();
	vr_renderWindowInteractor->Disable();
	renderWindow->OffScreenRenderingOn();
	renderWindow->SetDesiredUpdateRate(500.0);
	filter->SetInput(renderWindow);
	//filter->ShouldRerenderOff();
	texture->SetInputConnection(filter->GetOutputPort());
	texture2->SetInputData(image); // play with different textures to seperate data between threads
	panel->SetTexture(texture2);
	panel->Modified();
	renderWindow->Render();
}

vtkSmartPointer<vtkActor> vrep_vision_sensor::getNewactor() {
	vtkSmartPointer<vtkActor> newActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkImageData> newImage = vtkSmartPointer<vtkImageData>::New();
	// buildup
	vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
	vtkSmartPointer<vtkTextureMapToPlane> texturePlane = vtkSmartPointer<vtkTextureMapToPlane>::New();
	vtkSmartPointer<vtkPolyDataMapper> planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	vtkSmartPointer<vtkTransformFilter> transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
	vtkSmartPointer<vtkTexture> newTexture = vtkSmartPointer<vtkTexture>::New();
	transform->Scale(scaleX, scaleY, 1);
	transform->RotateZ(180);
	transform->RotateY(180);

	texturePlane->SetInputConnection(plane->GetOutputPort());
	transformFilter->SetInputConnection(texturePlane->GetOutputPort());

	transformFilter->SetTransform(transform);
	planeMapper->SetInputConnection(transformFilter->GetOutputPort());
	newActor->SetMapper(planeMapper);
	extraImages.push_back(newImage);
	newTexture->SetInputData(extraImages.back());
	newActor->SetTexture(newTexture);

	newActor->SetUserTransform(pose2); // this took long

	newActor->GetProperty()->SetAmbient(0.6);
	newActor->GetProperty()->SetDiffuse(0.4);
	newActor->GetProperty()->SetSpecular(1.0);
	newActor->GetProperty()->SetSpecularColor(0.25, 0.25, 0.25);
	newActor->Modified();
	return newActor;
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