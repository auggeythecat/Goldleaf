#pragma once
#include <base.hpp>
#include <fs/fs_Common.hpp>

namespace fs {
	namespace Archive {

		using ExtractStartCallback = std::function<void(const u64)>;
		using ExtractProgressCallback = std::function<void(const u64)>;
		
		bool isSupportedArchive(const std::string& path);	
		bool ExtractArchive(const std::string& archivePath, ExtractStartCallback start_cb, ExtractProgressCallback prog_cb);
		bool isSafePath(const std::string& path);
	};
}