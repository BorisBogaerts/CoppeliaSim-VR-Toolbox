#pragma once
#include <vtkLightActor.h>
#include <vtkLight.h>
#include "extApi.h"
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vector>

class vrep_light
{
public:
	vrep_light(int h, int rh, int cid);
	~vrep_light();
	void updatePose();
	void createLight(float color1[3], float color2[3], float angle, float exponent, float constAtten, float linAtten, float quadAtten);
	vtkSmartPointer<vtkLight> getLight() { return light[0]; };
	vtkSmartPointer<vtkLightActor> getActor() { return actor[0]; };
	vtkSmartPointer<vtkLight> getNewLight();
protected:
	int clientID;
	int handle;
	int refH;
	vtkSmartPointer<vtkTransform> pose = vtkSmartPointer<vtkTransform>::New();
	std::vector<vtkSmartPointer<vtkLight>> light;
	std::vector<vtkSmartPointer<vtkLightActor>> actor;
	
	float color1_mem[3];
	float color2_mem[3];
	float angle_mem;
	float exponent_mem;
	float constAtten_mem;
	float linAtten_mem;
	float quadAtten_mem;
};

