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

#include "vrep_controlled_object.h"
#include <cmath>
#include <dense>

#define PI 3.14f


vrep_controlled_object::vrep_controlled_object(int cid, int refH)
{
	clientID = cid;
	refHandle = refH;
}


vrep_controlled_object::~vrep_controlled_object()
{
}

void vrep_controlled_object::setName(std::string n) {
	name = n;
	simxGetObjectHandle(clientID, name.c_str(), &objectHandle, simx_opmode_blocking);
}

void vrep_controlled_object::updatePosition(vtkSmartPointer<vtkOpenVRRenderWindow> rw, vtkSmartPointer<vtkOpenVRRenderWindowInteractor> rwi, vtkSmartPointer<vtkOpenVRCamera> cam) {
	vtkSmartPointer<vtkTransform> Tt = vtkSmartPointer<vtkTransform>::New();
	Tt->PostMultiply();
	double orr[4];
	float pos2[3];

	if (device == vtkEventDataDevice::HeadMountedDisplay) {
		double *wxyz;
		wxyz = cam->GetOrientationWXYZ();
		Tt->RotateWXYZ(-wxyz[0], wxyz[1], wxyz[2], wxyz[3]); //fxcking -
		Tt->Translate(cam->GetPosition());
	}
	else {
		if (rw->GetTrackedDeviceModel(device) == nullptr) { return; }
		double pos[3];
		double ppos[3];
		double wdir[3];
		double wxyz[4];
		vr::TrackedDevicePose_t& vrPose = rw->GetTrackedDevicePose(rw->GetTrackedDeviceIndexForDevice(device));
		rwi->ConvertPoseToWorldCoordinates(vrPose, pos, wxyz, ppos, wdir);
		Tt->RotateWXYZ(wxyz[0], wxyz[1], wxyz[2], wxyz[3]);
		Tt->Translate(pos);
	}
	using ::Eigen::AngleAxisd;
	using ::Eigen::Matrix3d;
	using ::Eigen::Vector3d;

	Tt->RotateX(90);
	Tt->GetPosition(pos2);
	vtkSmartPointer<vtkTransform> textra = vtkSmartPointer<vtkTransform>::New();
	textra->Identity();
	textra->PostMultiply();
	textra->RotateX(-180);
	textra->RotateZ(-180);
	textra->Inverse();

	textra->Concatenate(Tt);
	textra->GetOrientationWXYZ(orr);

	
	double orrt[3];
	for (int i = 1; i < 4; i++) {
		orrt[i - 1] = orr[i];
	}
	Matrix3d R;
	AngleAxisd angleAxis(PI* orr[0]/180, Vector3d(orrt));
	R = AngleAxisd(angleAxis);
	Vector3d ea = R.eulerAngles(0,1,2);
	float temp[3];
	for (int i = 0; i < 3; i++) {
		temp[i] = ea[i];
	}
	simxSetObjectOrientation(clientID, objectHandle, refHandle, temp, simx_opmode_streaming);
	simxSetObjectPosition(clientID, objectHandle, refHandle, pos2, simx_opmode_streaming);
	//cout << pos[0] << " " << pos[1] << " " << pos[2] << " " << orr[0] << " " << orr[1] << " " << orr[2] << endl;
}

