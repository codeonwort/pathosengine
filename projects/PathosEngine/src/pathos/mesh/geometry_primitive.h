#pragma once

#include "geometry.h"
#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/types/enum.h"

namespace pathos {

	enum class EPrimitiveInitOptions : uint32 {
		None                     = 0,
		CalculatePosition        = 1 << 0,
		CalculateUV              = 1 << 1,
		CalculateNormal          = 1 << 2,
		CalculateTangentBasis    = 1 << 3,

		Default                  = CalculatePosition | CalculateUV | CalculateNormal | CalculateTangentBasis,
		All                      = 0xffffffff
	};
	ENUM_CLASS_FLAGS(EPrimitiveInitOptions);

	class PlaneGeometry : public MeshGeometry {

	public:
		// Determines surface normal direction.
		enum class Direction { X = 0, Y = 1, Z = 2 };

		// NOTE: CalculateTangentBasis flag in options is ignored.
		static void generate(
			float width, float height,
			uint32 gridX, uint32 gridY,
			PlaneGeometry::Direction direction,
			EPrimitiveInitOptions options,
			std::vector<float>& outPositions,
			std::vector<float>& outUVs,
			std::vector<float>& outNormals,
			std::vector<uint32>& outIndices);

		PlaneGeometry(
			float width, float height,
			uint32 gridX = 1, uint32 gridY = 1,
			PlaneGeometry::Direction direction = PlaneGeometry::Direction::Z,
			EPrimitiveInitOptions options = EPrimitiveInitOptions::Default);

		void buildGeometry(PlaneGeometry::Direction direction, EPrimitiveInitOptions options);

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
