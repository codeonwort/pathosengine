// #todo-gltf: GLTF does not support curves :/
// WON'T DO: Until needed.
#error "DO NOT INCLUDE IN PROJECT"

#pragma once

#include "badger/types/noncopyable.h"
#include <string>

namespace pathos {

	class GLTFLoader final : public Noncopyable {

	public:
		GLTFLoader() = default;
		~GLTFLoader() = default;

		bool loadASCII(const std::string& inFilename);

		bool containsMeshName(const char* inMeshName);

	private:
		//

	};

}
