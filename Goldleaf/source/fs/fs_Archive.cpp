//TODO: Do this better v
#include <fs/fs_Common.hpp>
#include <fs/fs_Archive.hpp>
#include <fs/fs_FileSystem.hpp>
#include <fs/fs_Common.hpp>
#include <archive.h>
#include <archive_entry.h>

namespace fs {
	bool Archive::isSupportedArchive(const std::string& path) {
		// check if its a full path or just a file name
		if (path[0] != '/') {	
			const auto ext = LowerCaseString(GetExtension(path));

			//TODO: add more accurate extension checks
			if (ext == "zip" || ext == "tar" || ext == "gz" || ext == "xz" || ext == "bz2" || ext == "7z" || ext == "rar") {
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


	bool Archive::ExtractArchive(const std::string& archivePath, ExtractStartCallback start_cb, ExtractProgressCallback prog_cb) {
		auto exp = fs::GetSdCardExplorer();
		auto totalAchiveSize = exp->GetFileSize(archivePath);

		start_cb(totalAchiveSize);

		std::string extractDir = archivePath.substr(0, archivePath.find_last_of("."));
		exp->CreateDirectory(extractDir);

		struct archive *a = archive_read_new();
		archive_read_support_format_all(a);
		archive_read_support_filter_all(a);

		if (archive_read_open_filename(a, archivePath.c_str(), 256 * 1024) != ARCHIVE_OK) { archive_read_free(a); return false; }

		struct archive_entry *entry;

		u64 last_header_offset = 0;

		while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
				std::string currentFile = extractDir + "/" + std::string(archive_entry_pathname(entry));
				if (!isSafePath(currentFile)) {
					continue;
				}
				
				if (archive_entry_filetype(entry) == AE_IFDIR) {
					exp->CreateDirectory(currentFile);
					GLEAF_LOG_FMT("Making directory: %s", currentFile.c_str());
					continue;
				}

				auto f = fopen(currentFile.c_str(), "wb");
				const void* workbuf;
				size_t size;
				u64 offset;
				u64 currentPos = 0;

				while (archive_read_data_block(a, &workbuf, &size, (int64_t*) &offset) == ARCHIVE_OK) {
					// exp->WriteFile(currentFile, workbuf, size);
					GLEAF_LOG_FMT("Writing file: %s (Offset: %ld, Size: %zu)", currentFile.c_str(), offset, size);
					
					if(offset != currentPos) {
						fseeko(f, offset, SEEK_SET);
						currentPos = offset;
					}

					currentPos += fwrite(workbuf, 1, size, f);
			
				}
				fclose(f);
				u64 current_offset = archive_read_header_position(a);

				if(current_offset > last_header_offset) {
					prog_cb(current_offset - last_header_offset);
					last_header_offset = current_offset;
				}
			}

		if(totalAchiveSize > last_header_offset) {
			prog_cb(totalAchiveSize - last_header_offset);
			
			archive_read_free(a);
			
		}
		return true;
	}
	
	bool Archive::isSafePath(const std::string& path) {
		if (path.find("..") != std::string::npos) {
			GLEAF_WARN_FMT("SKIPPED FILE. Unsafe path detected: %s", path.c_str());
			return false;
		}
		return true;
	}
}