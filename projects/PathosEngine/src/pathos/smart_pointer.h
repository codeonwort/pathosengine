#pragma once

#include <memory>

// It's far-off that I'll implement my own smart pointers, but here is minimal abstraction.

namespace pathos {

	template<typename T>
	using sharedPtr = std::shared_ptr<T>;

	template<typename T>
	using uniquePtr = std::unique_ptr<T>;

	template<typename T>
	using weakPtr = std::weak_ptr<T>;

	template<typename T, typename ...Args>
	uniquePtr<T> makeUnique(Args&& ...args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	sharedPtr<T> makeShared(Args&& ...args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}
