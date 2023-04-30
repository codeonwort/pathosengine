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

	// Elapsed milliseconds since start()
	float stop() {
		auto diff = std::chrono::system_clock::now() - startTime;
		std::chrono::microseconds elapsedMicro
			= std::chrono::duration_cast<std::chrono::microseconds>(diff);

		return 0.001f * (float)elapsedMicro.count();
	}

private:
	std::chrono::system_clock::time_point startTime;

};
