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
#include <vtkRendererCollection.h>
#include <vtkFrustumCoverageCuller.h>

void GetRayForODSCamera(double x, double y, int which_eye, double IPD, double* origin, double*direction) {
	double theta = (x * 2 * 3.1415) - 3.1415;
	double phi = (3.1415 / 2) - (y * 3.1415);
	double scale;
	if (which_eye == 0) { // left eye
		scale = -IPD / 2;
	}
	else {
		scale = IPD / 2;
	}
	/*origin[0] = cos(theta)*scale;
	origin[1] = 0;
	origin[2] = sin(theta)*scale;*/
	direction[0] = sin(theta) * cos(phi);
	direction[1] = sin(phi);
	direction[2] = cos(theta) * cos(phi);
}

stereoPanorama_renderwindow_support::~stereoPanorama_renderwindow_support()
{
}



void stereoPanorama_renderwindow_support::updateRender() {
	renderWindow->Render();
};

void stereoPanorama_renderwindow_support::Initialize() {
	renderWindow->SetDesiredUpdateRate(1000.0);
	
	// This starts the event loop and as a side effect causes an initial render.
	int width = 1;
	double totalWidth = 4096;
	renderWindow->SetSize(totalWidth, 512);
	
	double *origin = new double[3]{ 0,1,0 };
	double *direction = new double[3]{ 0,0,0 };
		for (int i = 0; i < width; i++) {
			GetRayForODSCamera((double)i / (double)width, 0.25, 0, 0.012, origin, direction);
			double*port = new double[4]{ ((double)i*(totalWidth / width)) / totalWidth ,0.0,(double(i + 1)*(totalWidth / width)) / totalWidth,1 };
			vtkSmartPointer<vtkOpenGLRenderer> renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
			vtkSmartPointer<vtkFrustumCoverageCuller> cull = vtkSmartPointer<vtkFrustumCoverageCuller>::New();
			renderer->ResetCamera();
			renderer->SetViewport(port);
			renderer->GetActiveCamera()->SetViewAngle(30);
			renderer->GetActiveCamera()->SetFocalPoint(direction);
			renderer->GetActiveCamera()->SetPosition(origin);
			renderer->SetBackground(1.0, 1.0, 1.0);
			renderer->UseShadowsOff();
			//renderer->DrawOff();
	/*		cull->SetMinimumCoverage(0.1);
			cull->SetSortingStyle(2);
			renderer->AddCuller(cull);*/
			renderWindow->AddRenderer(renderer);
		}
};

void stereoPanorama_renderwindow_support::addVrepScene(vrep_scene_content *vrepSceneIn) {
	vrepScene = vrepSceneIn;
	vrepScene->vrep_get_object_pose();
	Initialize();
	for (int i = 0; i < vrepScene->getNumActors(); i++) {
		vrepScene->getActor(i)->PickableOff();
		vrepScene->getActor(i)->GetProperty()->SetAmbient(0.7);
		vrepScene->getActor(i)->GetProperty()->SetDiffuse(0.5);
		
		renderWindow->GetRenderers()->InitTraversal();
		for (int ii = 0; ii < renderWindow->GetRenderers()->GetNumberOfItems(); ii++) {
			renderWindow->GetRenderers()->GetNextItem()->AddActor(vrepScene->getActor(i));
		}
	}

	for (int i = 0; i < vrepScene->getNumRenders(); i++) {
		renderWindow->GetRenderers()->InitTraversal();
		for (int ii = 0; ii < renderWindow->GetRenderers()->GetNumberOfItems(); ii++) {
			renderWindow->GetRenderers()->GetNextItem()->AddActor(vrepScene->getPanelActor(i));
		}
	}
	if (vrepScene->isVolumePresent()) {
		renderWindow->GetRenderers()->InitTraversal();
		for (int ii = 0; ii < renderWindow->GetRenderers()->GetNumberOfItems(); ii++) {
			renderWindow->GetRenderers()->GetNextItem()->AddViewProp(vrepScene->getVolume());
		}
		grid = vrepScene->vol;
		//renderer->ResetCamera();
	}
}

void stereoPanorama_renderwindow_support::updatePose() {
	vrepScene->updateMainCamObjectPose();

}

void stereoPanorama_renderwindow_support::syncData(vtkSmartPointer<vtkWin32OpenGLRenderWindow> win) {
	//vrepScene->transferVisionSensorData(iren, win, ren);
}



void stereoPanorama_renderwindow_support::visionSensorThread() {
	vrepScene->activateNewConnection(); // connect to vrep with a new port
	while (true) {
		vrepScene->updateVisionSensorObjectPose();
		coverage = vrepScene->updateVisionSensorRender();
		dataReady = true;
		//cout << "vision" << endl;
		while (dataReady) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		};
	}
}

void handleFunc(stereoPanorama_renderwindow_support *sup) {
	cout << "Vision sensor thread activated" << endl;
	sup->visionSensorThread();
}

void stereoPanorama_renderwindow_support::activate_interactor() {
	vrepScene->vrep_get_object_pose();


	cout << "Everithing loaded succesfully" << endl;
	renderWindow->Initialize();
	renderWindow->SetMultiSamples(0);
	//renderWindow->EraseOff();
	renderWindow->Render();

	std::thread camThread(handleFunc, this);
	while (true) {
		updatePose();
		renderWindow->Render();
		chrono->increment();
		//cout << "render" << endl;
		if (isReady()) {
			syncData(renderWindow);
			//scale = vr_renderWindowInteractor->GetPhysicalScale();
			chrono->increment2();
			setNotReady();
		}
		if (simxGetLastCmdTime(clientID) <= 0) {
			return;
		}
	}
}

