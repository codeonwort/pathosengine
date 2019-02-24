#pragma once

#include "geometry.h"
#include <stdint.h>

namespace pathos {

	class PlaneGeometry : public MeshGeometry {
	public:
		PlaneGeometry(float width, float height, uint32_t segW = 1, uint32_t segH = 1);
		void buildGeometry();
	private:
		float width, height;
		uint32_t gridX, gridY;
	};

	class CubeGeometry : public MeshGeometry {
	public:
		CubeGeometry(const glm::vec3& halfSize);
		void buildGeometry();
	private:
		glm::vec3 halfSize;
	};

	class SphereGeometry : public MeshGeometry {
	public:
		SphereGeometry(float radius, unsigned int division = 20);
		void buildGeometry();
	private:
		float radius;
		unsigned int division;
	};

}
