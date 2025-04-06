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
		struct Input {
			float width;
			float height;
			uint32 gridX = 1; // number of cells in x-axis
			uint32 gridY = 1; // number of cells in y-axis
			PlaneGeometry::Direction direction = PlaneGeometry::Direction::Z;
			EPrimitiveInitOptions options = EPrimitiveInitOptions::Default;
		};
		struct Output {
			std::vector<float> positions;
			std::vector<float> texcoords;
			std::vector<float> normals;
			std::vector<uint32> indices;
		};

		// NOTE: CalculateTangentBasis flag in input.options is ignored.
		static void generate(const Input& input, Output& output);

		PlaneGeometry(const Input& input);
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
		struct Input {
			float radius;
			uint32 subdivision = 20;
			EPrimitiveInitOptions options = EPrimitiveInitOptions::Default;
		};
		struct Output {
			std::vector<float> positions;
			std::vector<float> texcoords;
			std::vector<float> normals;
			std::vector<uint32> indices;
		};
		// NOTE: CalculateTangentBasis flag in input.options is ignored.
		static void generate(const Input& input, Output& output);

		SphereGeometry(const Input& input);
	};

}
