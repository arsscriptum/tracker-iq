
#include <iostream>
#include <string>

namespace dhtd {
	class version {
	public:
		static unsigned int major;
		static unsigned int minor;
		static unsigned int build;
		static std::string  sha; 
		static std::string  branch;

	};

}