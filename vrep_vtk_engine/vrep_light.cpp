#include "vrep_light.h"



vrep_light::vrep_light(int h, int rh, int cid)
{
	clientID = cid;
	handle = h;
	refH = rh;
	light.push_back(vtkSmartPointer<vtkLight>::New());
	actor.push_back(vtkSmartPointer<vtkLightActor>::New());
}


vrep_light::~vrep_light()
{
}

void vrep_light::createLight(float color1[3], float color2[3], float angle, float exponent, float constAtten, float linAtten, float quadAtten) {
	// store for later use
	color1_mem[0] = color1[0];
	color1_mem[1] = color1[1];
	color1_mem[2] = color1[2];
	color2_mem[0] = color2[0];
	color2_mem[1] = color2[1];
	color2_mem[2] = color2[2];
	angle_mem = angle;
	exponent_mem = exponent;
	constAtten_mem = constAtten;
	linAtten_mem = linAtten;
	quadAtten_mem = quadAtten;
	
	light[0]->SetPositional(true);
	light[0]->SetPosition(0, 0, 0);
	light[0]->SetFocalPoint(0, 0, 1);

	light[0]->SetDiffuseColor(color1[0], color1[1], color1[2]);
	light[0]->SetSpecularColor(color2[0], color2[1], color2[2]);
	light[0]->SetConeAngle(angle);
	light[0]->SetExponent(exponent);
	light[0]->SetAttenuationValues(constAtten, linAtten, quadAtten);
	light[0]->SetTransformMatrix(pose->GetMatrix());
	light[0]->Modified();
	actor[0]->SetLight(light[0]);
}

vtkSmartPointer<vtkLight> vrep_light::getNewLight() {
	light.push_back(vtkSmartPointer<vtkLight>::New());
	actor.push_back(vtkSmartPointer<vtkLightActor>::New());
	
	light[light.size()-1]->SetPositional(true);
	light[light.size()-1]->SetPosition(0, 0, 0);
	light[light.size()-1]->SetFocalPoint(0, 0, 1);

	light[light.size()-1]->SetDiffuseColor(color1_mem[0], color1_mem[1], color1_mem[2]);
	light[light.size()-1]->SetSpecularColor(color2_mem[0], color2_mem[1], color2_mem[2]);
	light[light.size()-1]->SetConeAngle(angle_mem);
	light[light.size()-1]->SetExponent(exponent_mem);
	light[light.size()-1]->SetAttenuationValues(constAtten_mem, linAtten_mem, quadAtten_mem);
	light[light.size()-1]->SetTransformMatrix(pose->GetMatrix());
	light[light.size()-1]->Modified();
	actor[light.size()-1]->SetLight(light[light.size()-1]);
	return light[light.size()-1];
}

void vrep_light::updatePose() {
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

	//pose->Inverse();
	pose->Modified();
	light[0]->SetTransformMatrix(pose->GetMatrix());
	light[0]->Modified();
	for (int i = 1; i<light.size(); i++){
		vtkSmartPointer<vtkTransform> newPose = vtkSmartPointer<vtkTransform>::New();
		newPose->DeepCopy(pose);
		light[i]->SetTransformMatrix(newPose->GetMatrix());
		light[i]->Modified();
	}
}