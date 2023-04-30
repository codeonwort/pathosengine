#pragma once

#include <chrono>

struct Stopwatch {

public:
	Stopwatch() {
		startTime = std::chrono::system_clock::now();
	}

	inline void start() {
		startTime = std::chrono::system_clock::now();
	}

	// Elapsed milliseconds since start()
	inline float stop() const {
		auto diff = std::chrono::system_clock::now() - startTime;
		std::chrono::microseconds elapsedMicro
			= std::chrono::duration_cast<std::chrono::microseconds>(diff);

		return 0.001f * (float)elapsedMicro.count();
	}

private:
	std::chrono::system_clock::time_point startTime;

};
