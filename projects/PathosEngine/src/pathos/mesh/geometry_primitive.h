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
		// Determines surface normal direction.
		enum class Direction { X = 0, Y = 1, Z = 2 };
		PlaneGeometry(
			float width, float height,
			uint32 segW = 1, uint32 segH = 1,
			PlaneGeometry::Direction direction = PlaneGeometry::Direction::Z,
			EPrimitiveInitOptions options = EPrimitiveInitOptions::Default);
		void buildGeometry(PlaneGeometry::Direction direction);
	private:
		float width, height;
		uint32 gridX, gridY;
	};

	class CubeGeometry : public MeshGeometry {
	public:
		CubeGeometry(
			const vector3& halfSize,
			bool smoothing = false,
			EPrimitiveInitOptions options = EPrimitiveInitOptions::Default);
		void buildGeometry(bool smoothing);
	private:
		void buildGeometry_smoothing();
		void buildGeometry_noSmoothing();
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
