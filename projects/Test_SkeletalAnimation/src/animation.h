#pragma once

#include "assimp/scene.h"

#include <string>
#include <vector>

namespace pathos {

	class SkeletalAnimation {

	public:
		SkeletalAnimation(aiAnimation* anim) { this->anim = anim; }

		inline const std::string getName() const { return std::string(anim->mName.C_Str()); }
		double getLength() const { return anim->mDuration; }

		// TODO: encalsulate aiAnimation
	//private:
	public:
		aiAnimation* anim;

	};

}