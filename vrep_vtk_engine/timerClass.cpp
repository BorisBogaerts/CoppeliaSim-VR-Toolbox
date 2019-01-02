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

#include "timerClass.h"
#include <sstream>
using namespace std;
void timerClass::increment() {
	if (counter == 0) {
		t1 = Clock::now();
	}; counter++;

	if (counter == countInterval) {
		counter = 0;
		std::chrono::steady_clock::time_point tn = Clock::now();
		time = (std::chrono::duration_cast<std::chrono::milliseconds>(tn - t1).count());
		time = (1000 * countInterval) / (time);
		float temp;
		if (scale == nullptr) { temp = 1; }
		else { temp = *scale; };
		if (mode == 0) {
			std::cout << '\r' << "fps: " << (int)time << "     Scale: " << 100.0 / temp << "%";
			stringstream ss;
			ss << "Fps: " << (int)time << " \nScale: " << 100.0 / temp << "%";
			text = ss.str();
		}
		else if (mode == 1) {
			std::cout << '\r' << "fps: " << (int)time << "     time thread 2 (ms): " << time2 << "     Coverage: " << int((*coverage) * 100) << "%     Scale: " << 100.0 / temp << "%";
			stringstream ss;
			ss << "Fps: " << (int)time << "\nThread 2 (ms): " << time2 << "\nCoverage: " << int((*coverage) * 100) << "% \nScale: " << 100.0 / temp << "%";
			text = ss.str();
		}

	}
}

void timerClass::increment2() {
	mode = 1;
	if (counter2 == 0) {
		t2 = Clock::now();
	}; counter2++;

	if (counter2 == countInterval2) {
		counter2 = 0;
		std::chrono::steady_clock::time_point tn = Clock::now();
		time2 = (std::chrono::duration_cast<std::chrono::milliseconds>(tn - t2).count()) / countInterval2;
		float temp = *scale;
		std::cout << '\r' << "fps: " << (int)time << "     time thread 2 (ms): " << time2 << "     Coverage: " << int((*coverage) * 100) << "%     Scale: " << 100.0 / temp << "%";
		stringstream ss;
		ss << "Fps: " << (int)time << "\nThread 2 (ms): " << time2 << "\nCoverage: " << int((*coverage) * 100) << "% \nScale: " << 100.0 / temp << "%";
		text = ss.str();
	}
}
