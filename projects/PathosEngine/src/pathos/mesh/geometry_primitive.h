#pragma once

#include "geometry.h"

namespace pathos {

	class PlaneGeometry : public MeshGeometry {
	private:
		float width, height;
		unsigned int gridX, gridY;
	public:
		PlaneGeometry(float width, float height, unsigned int segW = 1, unsigned int segH = 1);
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