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
		float temp = *scale;
		std::cout << '\r' << "fps: " << (int)time << "     time thread 2 (ms): " << time2 << "     Coverage: " << int((*coverage) * 100) << "%     Scale: " << 100.0 / temp << "%";
		stringstream ss;
		ss << "Fps: " << (int)time << "\nThread 2 (ms): " << time2 << "\nCoverage: " << int((*coverage) * 100) << "% \nScale: " << 100.0 / temp << "%";
		text = ss.str();
	}
}

void timerClass::increment2() {
	if (counter2 == 0) {
		t2 = Clock::now();
	}; counter2++;

	if (counter2 == countInterval2) {
		counter2 = 0;
		std::chrono::steady_clock::time_point tn = Clock::now();
		time2 = (std::chrono::duration_cast<std::chrono::milliseconds>(tn - t2).count())/ countInterval2;
		float temp = *scale;
		std::cout << '\r' << "fps: " << (int)time << "     time thread 2 (ms): " << time2 << "     Coverage: " << int((*coverage) * 100) << "%     Scale: " << 100.0/temp << "%";
		stringstream ss;
		ss << "Fps: " << (int)time << "\nThread 2 (ms): " << time2 << "\nCoverage: " << int((*coverage) * 100) << "% \nScale: " << 100.0 / temp << "%";
		text = ss.str();
	}
}