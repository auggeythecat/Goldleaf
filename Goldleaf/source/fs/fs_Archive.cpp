//TODO: Do this better v
#include <fs/fs_Common.hpp>
#include <fs/fs_Archive.hpp>
#include <archive.h>

namespace fs {
	bool Archive::isSupportedArchive(const std::string& path) {
		// check if its a full path or just a file name
		if (path[0] != '/') {	
			const auto ext = LowerCaseString(GetExtension(path));

			//TODO: add more accurate extension checks
			if (ext == "zip" || ext == "tar" || ext == "gz" || ext == "xz" || ext == "bz2") {
				return true;
			}
		}

		// if it is a full path, use the good library functions for this purpose

		struct archive* a = archive_read_new();
		archive_read_support_format_all(a);
		archive_read_support_filter_all(a);

		int r = archive_read_open_filename(a, path.c_str(), 10240);

		if (r != ARCHIVE_OK) {
			archive_read_free(a);
			return false;
		}

		struct archive_entry *entry;
		r = archive_read_next_header(a, &entry);

		archive_read_close(a);
		archive_read_free(a);

		return (r == ARCHIVE_OK || r == ARCHIVE_EOF);
	}
}