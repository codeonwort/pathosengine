#pragma once

namespace badger {

	template<typename T>
	T min(T x, T y) {
		return (x < y) ? x : y;
	}

	template<typename T>
	T max(T x, T y) {
		return (x < y) ? y : x;
	}

	// #todo-math: Exotic order? Is (value, minValue, maxValue) more common?
	template<typename T>
	T clamp(T minValue, T value, T maxValue) {
		return (value < minValue) ? minValue : value > maxValue ? maxValue : value;
	}

}
