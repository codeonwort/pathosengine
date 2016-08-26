#include "core.h"

namespace pathos {

	const std::string& NamedObject::getName() const { return name; }
	void NamedObject::setName(const std::string& n) { name = n; }

}