#pragma once

#include "geometry.h"
#include <stdint.h>

namespace pathos {

	class PlaneGeometry : public MeshGeometry {
	private:
		float width, height;
		uint32_t gridX, gridY;
	public:
		PlaneGeometry(float width, float height, uint32_t segW = 1, uint32_t segH = 1);
		virtual void buildGeometry();
	};

	class CubeGeometry : public MeshGeometry {
	private:
		glm::vec3 halfSize;
	public:
		CubeGeometry(const glm::vec3& halfSize);
		virtual void buildGeometry();
	};

	class SphereGeometry : public MeshGeometry {
	private:
		float radius;
		unsigned int division;
	public:
		SphereGeometry(float radius, unsigned int division = 20);
		virtual void buildGeometry();
	};

}