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

#include "extApi.h"
#include <iostream>
#include "vrep_scene_content.h"
#include "vrep_controlled_object.h"
#include "vrep_volume_grid.h"
#include "renderwindow_support.h"
// vr renderwindow

using namespace std;
int main()
{
	//	 First make a connection to vrep
	int portNb = 19997;
	int clientID = -1;
	int clientID2 = -1;
	int running = 0;

	cout << "Trying to connect with V-REP ..." << endl;
	while (clientID == -1) {
		Sleep(100);
		clientID = simxStart((simxChar*)"127.0.0.1", portNb, true, true, -200000, 5);
	}

	cout << endl << "Connected to V-REP, clientID number : " << clientID << endl;
	cout << "Connected to V-REP port number : " << portNb << endl << endl;

	simxInt *data;
	simxInt dataLength;
	simxInt result;
	int interactor;
	while (true) {
		// Wait until simulation gets activated
		cout << "Wait for simulation start ..." << endl;
		result = 8;
		while ((result == 8) || (running == 0)) {
			result = simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"isRunning"
				, 0, NULL, 0, NULL, 0, NULL, 0, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);
			if (result != 8) {
				running = data[0];
				interactor = data[1];
			}
		}
		cout << "Simulation started" << endl << endl;
		int refHandle;

		simxGetObjectHandle(clientID, (simxChar*)"HTC_VIVE", &refHandle, simx_opmode_blocking);
		vrep_scene_content *scene = new vrep_scene_content(clientID, refHandle); // all geometry is relative to refference frame
		scene->loadVolume();
		scene->loadScene(true); // read scene from VREP
		scene->loadCams();
		scene->connectCamsToVolume();
		scene->checkMeasurementObject(); // if the volume is replaced by a mesh then we need to change a few things
		renderwindow_support *supp = new renderwindow_support(clientID, refHandle, interactor);
		supp->addVrepScene(scene);
		supp->activate_interactor();
		delete supp;
		delete scene;
		Sleep(500);
	}
	return EXIT_SUCCESS;
}
