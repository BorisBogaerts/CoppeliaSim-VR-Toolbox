#include <chrono>
#include <ctime>
#include <string>
#include <iostream>

#pragma once
typedef std::chrono::high_resolution_clock Clock;

class timerClass {
public:
	void set_interval(int interval) { countInterval = interval; std::cout << "fps: 0"; };
	void increment();
	void increment2();
	float *coverage;
	float *scale;
private:
	int counter = 0;
	int countInterval = 50;

	int counter2 = 0;
	int countInterval2 = 10;
	double time, time2;
	std::chrono::steady_clock::time_point t1;
	std::chrono::steady_clock::time_point t2;
};

