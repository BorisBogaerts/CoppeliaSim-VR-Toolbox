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

#pragma once

#include "stereoPanorama_renderwindow_support.h"
#include <vector>
#include "extApi.h"
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>
#include <thread>
#include <chrono>
#include <vtkOpenGLPolyDataMapper.h>

#include <vtkOutputWindow.h>
#include <vtkFileOutputWindow.h>

void handleFunc(stereoPanorama_renderwindow_support *sup) {
	cout << "Vision sensor thread activated" << endl;
	sup->visionSensorThread();
}

stereoPanorama_renderwindow_support::stereoPanorama_renderwindow_support(int cid, int ref, int interactor)
{
	refH = ref;
	setClientID(cid, interactor);
	chrono->coverage = &coverage;
	// Now add spectator camera
	simxFloat *data;
	simxInt dataLength;

	simxGetObjectHandle(clientID, (simxChar*)"Vision_sensor", &handle, simx_opmode_blocking);

	vtkOutputWindow* ow = vtkOutputWindow::GetInstance();
	vtkFileOutputWindow* fow = vtkFileOutputWindow::New();
	fow->SetFileName("debug.log");
	if (ow)
	{
		ow->SetInstance(fow);
	}
	fow->Delete();
}

stereoPanorama_renderwindow_support::~stereoPanorama_renderwindow_support()
{

}

void stereoPanorama_renderwindow_support::updateRender() {
	vr_renderWindowInteractor->Render();
};


void stereoPanorama_renderwindow_support::addVrepScene(vrep_scene_content *vrepSceneIn) {
	vrepScene = vrepSceneIn;

	vrepScene->vrep_get_object_pose();
	//for (int i = 0; i < 1; i++) { //vrepScene->getNumActors()
	//	vrepScene->getActor(i)->PickableOff();
	//	vrepScene->getActor(i)->GetProperty()->SetAmbient(0.7);
	//	vrepScene->getActor(i)->GetProperty()->SetDiffuse(0.5);
	//	renderer->AddActor(vrepScene->getActor(i));
	//}
	////for (int i = 0; i < vrepScene->getNumRenders(); i++) {
	////	for (int j = 0; j < renderer.size(); j++) {
	////		renderer[j]->AddActor(vrepScene->getPanelActor(i));
	////	}
	////}
	//if (vrepScene->isVolumePresent()) {
	//	renderer->AddViewProp(vrepScene->getVolume());
	//	grid = vrepScene->vol;
	//	//renderer->ResetCamera();
	//}
	//else {
	//	grid = nullptr;
	//}
	grid = nullptr;
}

void stereoPanorama_renderwindow_support::updatePose() {
	vrepScene->updateMainCamObjectPose();

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
	vr_camera->SetModelTransformMatrix(pose->GetMatrix());
	vr_camera->Modified();
}

void stereoPanorama_renderwindow_support::syncData() {
	//vrepScene->transferVisionSensorData(iren, win, ren);
	vrepScene->transferVisionSensorData();
}

void stereoPanorama_renderwindow_support::visionSensorThread() {
	vrepScene->activateNewConnection(); // connect to vrep with a new port
	while (true) {
		vrepScene->updateVisionSensorObjectPose();
		coverage = vrepScene->updateVisionSensorRender();
		dataReady = true;
		while (dataReady) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		};
	}
}




void stereoPanorama_renderwindow_support::activate_interactor() {
	vrepScene->vrep_get_object_pose();

	vr_camera->SetViewAngle(90.0);
	vr_camera->SetPosition(0, 0, 0);
	vr_camera->SetFocalPoint(0, 0, 1);
	vr_camera->SetViewUp(0, 1, 0);

	// Classical vtk pipeline to set up renderwindow etc with extra options
	renderer->SetActiveCamera(vr_camera);
	addPlane(); // Used to obtain pixel coordinate in shader
	renderer->UseShadowsOff();
	renderer->Modified();

	renderWindow->AddRenderer(renderer);
	renderWindow->SetDesiredUpdateRate(90.0);
	renderWindow->SetSize(512, 512);
	renderWindow->Initialize();
	vr_renderWindowInteractor->SetRenderWindow(renderWindow);
	vr_renderWindowInteractor->Initialize();


	if ((grid != nullptr) && (vrepScene->isVolumePresent())) {
		cout << "Volume detected and connected" << endl;
	}

	cout << "Everithing loaded succesfully" << endl;


	// Search for dynamic path object
	path = new pathObject(clientID);
	renderer->AddActor(path->getActor());
	renderer->Modified();



	// Manage vision sensor thread (if necessary)
	std::thread camThread;
	if (vrepScene->startVisionSensorThread()) {
		camThread = std::thread(handleFunc, this);
	}
	else {
		camThread.~thread();
	}
	while (true) {
		updateRender();
		chrono->increment();

		path->update();
		if (isReady()) {
			syncData();
			//grid->updatMap();
			chrono->increment2();
			setNotReady();
		}

		if (simxGetLastCmdTime(clientID) <= 0) {
			vr_renderWindowInteractor->TerminateApp(); // stop if the v-rep simulation is not running
			camThread.~thread();
			return;
		}
	}
}


void stereoPanorama_renderwindow_support::addPlane() {
	vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
	plane->SetCenter(0.0, 0.0, 0.5);
	plane->SetNormal(0.0, 0.0, 1.0);

	vtkSmartPointer<vtkTextureMapToPlane> texturePlane = vtkSmartPointer<vtkTextureMapToPlane>::New();
	texturePlane->SetInputConnection(plane->GetOutputPort());

	vtkSmartPointer<vtkOpenGLPolyDataMapper> planeMapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
	planeMapper->SetInputConnection(texturePlane->GetOutputPort());

	vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();

	// Load custom shader code as string
	std::ifstream ifs("omnidirectional_shader.glsl");
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	std::string toInject;

	toInject.append("//VTK::Normal::Impl\n"); // vtk wants this
	toInject.append(content); // add shader code
	toInject.append("  return;\n"); // escape from shader
	planeMapper->AddShaderReplacement(vtkShader::Fragment,"//VTK::Normal::Impl", true, toInject, false); // inject custom shader code
	// Now remove a bunch of stuff (we don't want lights for example to interfere with any
	planeMapper->AddShaderReplacement(vtkShader::Fragment, "//VTK::Color::Dec", true, "", false); // Remove colors
	planeMapper->AddShaderReplacement(vtkShader::Fragment, "//VTK::Light::Dec", true, "", false); // Remove lights
	planeMapper->AddShaderReplacement(vtkShader::Fragment, "//VTK::Color::Impl", true, "", false); // Remove colors
	planeMapper->AddShaderReplacement(vtkShader::Fragment, "//VTK::Light::Impl", true, "", false); // Remove lights
	// same for vertex shader
	planeMapper->AddShaderReplacement(vtkShader::Vertex, "//VTK::Color::Dec", true, "", false); // Remove colors
	planeMapper->AddShaderReplacement(vtkShader::Vertex, "//VTK::Light::Dec", true, "", false); // Remove lights
	planeMapper->AddShaderReplacement(vtkShader::Vertex, "//VTK::Color::Impl", true, "", false); // Remove colors
	planeMapper->AddShaderReplacement(vtkShader::Vertex, "//VTK::Light::Impl", true, "", false);// Remove lights

	// Ok this is seriously dangerous summary:
		// I do not initialize the image properly
		// But I do not acces it in the shader, no problem right?
	vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
	image->SetDimensions(1, 1, 1);
	image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
	texture->SetInputData(image);
	vtkSmartPointer<vtkActor> texturedPlane = vtkSmartPointer<vtkActor>::New();
	texturedPlane->SetMapper(planeMapper);
	texturedPlane->SetTexture(texture);
	renderer->AddActor(texturedPlane);
}
