#pragma once

#include <string>

namespace pathos {
	
	class NamedObject {
	
	public:
		const std::string& getName() const;
		void setName(const std::string& n);

	private:
		std::string name = "";

	};

}