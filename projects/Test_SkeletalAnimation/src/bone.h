#pragma once

#include <string>
#include <vector>

namespace pathos {

	struct Bone {

	public:
		std::string name;
		std::vector<float> weights;
		std::vector<unsigned int> vertexIDs;
		glm::mat4 offset;
	};

}