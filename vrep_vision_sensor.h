#pragma once

#include <vtkSmartPointer.h>
#include <vtkWin32OpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkWin32RenderWindowInteractor.h>
#include <vtkOpenGLCamera.h>
#include <vtkTransform.h>
#include <vtkWindowToImageFilter.h>
#include <vtkImageData.h>
#include <vtkActor.h>
#include <vtkTexture.h>
#include <vtkPoints.h>

#include <vtkIdFilter.h>
#include <vtkTransformFilter.h>
#include <vtkSelectVisiblePoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>

#include <vtkPlaneSource.h>
#include <vtkTextureMapToPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkCameraActor.h>
class vrep_vision_sensor
{
public:
	vrep_vision_sensor() {};
	~vrep_vision_sensor() {};
	void updateRender();
	void setCameraParams(int h, float cameraParams[]);
	void updatePosition();
	void setClientID(int cid) { clientID = cid; };
	void activate(double scale[]);
	void activateBasic();
	void setPanel(vtkSmartPointer<vtkActor> pnl) { panel = pnl; };
	vtkSmartPointer<vtkOpenGLRenderer> getRenderer() { return renderer; };
	vtkSmartPointer<vtkActor> getActor() { return panel; };
	vtkSmartPointer<vtkPolyData> checkVisibility();
	vtkSmartPointer<vtkCameraActor> getCameraActor() { return camAct; };
	void setPointData(vtkSmartPointer<vtkPoints> data, vtkSmartPointer<vtkTransform> pose);
	void updatePose();
	void transferImageTexture();
private:
	int clientID;
	int handle;
	int update = 10;
	bool basic = false;
	vtkSmartPointer<vtkOpenGLRenderer> renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
	vtkSmartPointer<vtkWin32OpenGLRenderWindow> renderWindow = vtkSmartPointer<vtkWin32OpenGLRenderWindow>::New();
	vtkSmartPointer<vtkWin32RenderWindowInteractor> vr_renderWindowInteractor = vtkSmartPointer<vtkWin32RenderWindowInteractor>::New();
	vtkSmartPointer<vtkOpenGLCamera> vr_camera = vtkSmartPointer<vtkOpenGLCamera>::New();
	vtkSmartPointer<vtkTransform> invPose = vtkSmartPointer<vtkTransform>::New();
	vtkSmartPointer<vtkWindowToImageFilter> filter = vtkSmartPointer<vtkWindowToImageFilter>::New();
	vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
	vtkSmartPointer<vtkActor> panel = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();

	vtkSmartPointer<vtkTransformFilter > ptsT = vtkSmartPointer<vtkTransformFilter >::New();
	vtkSmartPointer<vtkSelectVisiblePoints> selectVisiblePoints = vtkSmartPointer<vtkSelectVisiblePoints>::New();
	vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkPolyData> ptset = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkIdFilter> ids = vtkSmartPointer<vtkIdFilter>::New();
	vtkSmartPointer<vtkTransform> pose = vtkSmartPointer<vtkTransform>::New();
	vtkSmartPointer<vtkCameraActor> camAct = vtkSmartPointer<vtkCameraActor>::New();
};

