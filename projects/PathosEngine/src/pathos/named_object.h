#pragma once

#include <string>

namespace pathos {
	
	// #todo: Is this really needed?
	class NamedObject {
	
	public:
		const std::string& getName() const;
		void setName(const std::string& n);

	private:
		std::string name = "";

	};

}
