#include "spline_loader.h"
#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"
#include <fstream>

namespace pathos {

	bool SplineLoader::load(const char* inFilename, bool bMakeLoop, std::string& outName, HermiteSpline& outSpline) {
		std::string filename = ResourceFinder::get().find(inFilename);

		uint32 numPoints;
		std::vector<vector3> points;
		std::vector<vector3> left_handles;
		std::vector<vector3> right_handles;

		std::fstream fp(filename, std::ios::in);
		if (!fp) {
			return false;
		}

		fp >> outName;
		fp >> numPoints;

		points.resize(numPoints);
		left_handles.resize(numPoints);
		right_handles.resize(numPoints);

		for (uint32 i = 0; i < numPoints; ++i) {
			fp >> points[i].x >> points[i].y >> points[i].z;
			fp >> left_handles[i].x >> left_handles[i].y >> left_handles[i].z;
			fp >> right_handles[i].x >> right_handles[i].y >> right_handles[i].z;
		}

		if (fp.bad()) {
			LOG(LogError, "Failed to parse: %s", filename);
			return false;
		}

		outSpline.clearPoints();
		for (uint32 i = 0; i < numPoints; ++i) {
			// #todo-spline: what to do with left handles?
			outSpline.addPoint(points[i], right_handles[i] - points[i]);
		}
		if (bMakeLoop) {
			outSpline.addPoint(points[0], right_handles[0] - points[0]);
		}
		outSpline.updateSpline();

		return true;
	}

}
