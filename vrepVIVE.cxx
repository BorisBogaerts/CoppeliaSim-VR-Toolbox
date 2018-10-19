// Copyright (c) 2017, Boris Bogaerts
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
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "vrep_scene_content.h"
#include "vrep_controlled_object.h"
#include <vtkVersion.h>
#include "vrep_volume_grid.h"
#include "vr_renderwindow_support.h"
#include "renderwindow_support.h"
#include "stereoPanorama_renderwindow_support.h"
// vr renderwindow

int main()
{
//	 First make a connection to vrep
	int portNb = 19997;
	int clientID = -1;
	int clientID2 = -1;
	int time = 0; 
		// Try to connect to V-REP
		
		std::cout << vtkVersion::GetVTKSourceVersion() << std::endl;
		cout << endl << "Trying to connect with V-REP ..." << endl;
		while (clientID == -1) {
			Sleep(100);
			clientID = simxStart((simxChar*)"127.0.0.1", portNb, true, true, -200000, 5);
		}

		cout << endl << "Connected to V-REP, clientID number : " << clientID << endl;
		cout << "Connected to V-REP port number : " << portNb  << endl << endl;
		
		// Wait until simulation gets activated
		cout << "Wait for simulation start ..." << endl;
		int temp;
		while (time == 0) {	
			temp = simxAddStatusbarMessage(clientID, (simxChar*)"Start simulation to proceed", simx_opmode_oneshot); // update time
			Sleep(500);
			time = simxGetLastCmdTime(clientID);
		}
		cout << "Simulation started" << endl << endl;
	
		vrep_scene_content *scene = new vrep_scene_content(clientID, -1); // all geometry is relative to refference frame
		scene->loadVolume();
		scene->loadScene(true); // read scene from VREP
		scene->loadCams();
		scene->connectCamsToVolume();
		vr_renderwindow_support *supp = new vr_renderwindow_support(clientID);
		supp->addVrepScene(scene);
		supp->activate_interactor(); 
	return EXIT_SUCCESS;
}
