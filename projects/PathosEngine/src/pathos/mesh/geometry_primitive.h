#pragma once

#include "geometry.h"
#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"

namespace pathos {

	enum EPrimitiveInitOptions : uint32 {
		None                     = 0,
		CalculateTangentBasis    = 1 << 0,

		Default                  = CalculateTangentBasis,
		All                      = 0xffffffff
	};

	class PlaneGeometry : public MeshGeometry {
	public:
		PlaneGeometry(float width, float height, uint32 segW = 1, uint32 segH = 1, EPrimitiveInitOptions options = EPrimitiveInitOptions::Default);
		void buildGeometry();
	private:
		float width, height;
		uint32 gridX, gridY;
	};

	class CubeGeometry : public MeshGeometry {
	public:
		CubeGeometry(const vector3& halfSize, EPrimitiveInitOptions options = EPrimitiveInitOptions::Default);
		void buildGeometry();
	private:
		vector3 halfSize;
	};

	class SphereGeometry : public MeshGeometry {
	public:
		SphereGeometry(float radius, uint32 division = 20, EPrimitiveInitOptions options = EPrimitiveInitOptions::Default);
		void buildGeometry();
	private:
		float radius;
		uint32 division;
	};

}
