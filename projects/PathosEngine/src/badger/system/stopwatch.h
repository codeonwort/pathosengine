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

	// Elapsed seconds since start()
	float stop() {
		auto diff = std::chrono::system_clock::now() - startTime;
		std::chrono::milliseconds elapsedMS
			= std::chrono::duration_cast<std::chrono::milliseconds>(diff);

		return 0.001f * (float)elapsedMS.count();
	}

private:
	std::chrono::system_clock::time_point startTime;

};
