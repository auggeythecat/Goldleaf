#pragma once
#include <base.hpp>

namespace fs {
	namespace Archive {
		bool isSupportedArchive(const std::string& path);	
		bool ExtractArchive(const std::string& archivePath);
		bool isSafePath(const std::string& path);
	};
}