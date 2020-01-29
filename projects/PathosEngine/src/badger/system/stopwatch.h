#pragma once

#include <chrono>

struct Stopwatch {
	
public:
	Stopwatch() {
		startTime = std::chrono::system_clock::now();
	}

	void start() {
		startTime = std::chrono::system_clock::now();
	}

	float stop() {
		std::chrono::duration<float> diff = std::chrono::system_clock::now() - startTime;
		float seconds = diff.count();

		return seconds;
	}

private:
	std::chrono::system_clock::time_point startTime;

};
