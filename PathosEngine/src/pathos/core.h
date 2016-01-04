#pragma once

#include <string>

namespace pathos {
	
	class NamedObject {
	protected:
		std::string name = "";
	public:
		const std::string& getName() const;
		void setName(const std::string& n);
	};

}