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

	simxGetObjectOrientation(clientID, handle, -1, eulerAngles, simx_opmode_streaming); // later replace by : simx_opmode_buffer 
	simxGetObjectPosition(clientID, handle, -1, position, simx_opmode_streaming);
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
	this->updatePosition();
	ptsT->SetTransform(pose);

	selectVisiblePoints->SetTolerance(0);
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
		filter->Modified();
		filter->ReadFrontBufferOff();
		filter->Update();
	}
};

void vrep_vision_sensor::transferImageTexture() {
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
	texture->SetInputData(image);
	panel->SetTexture(texture);
	panel->Modified();

	camAct->SetCamera(vr_camera);
	camAct->PickableOff();
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
//vr_camera->SetPosition(pose->GetPosition());

//vr_camera->SetFocalPoint(pose->GetMatrix()->GetElement(0, 3) - (focalLength * pose->GetMatrix()->GetElement(0, 2)),
//	pose->GetMatrix()->GetElement(1, 3) - (focalLength * pose->GetMatrix()->GetElement(1, 2)),
//	pose->GetMatrix()->GetElement(2, 3) - (focalLength * pose->GetMatrix()->GetElement(2, 2)));

//vr_camera->SetViewUp(pose->GetMatrix()->GetElement(0, 1), pose->GetMatrix()->GetElement(1, 1), pose->GetMatrix()->GetElement(2, 1));