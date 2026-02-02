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


	bool Archive::ExtractArchive(const std::string& archive_path, ExtractStartCallback start_cb, ExtractProgressCallback prog_cb) {
		auto exp = fs::GetSdCardExplorer();
		
		auto total_archive_size = exp->GetFileSize(archive_path);
		start_cb(total_archive_size);

		std::string extract_dir = archive_path;
		size_t dot_pos = extract_dir.rfind(".");

		if (dot_pos != std::string::npos && dot_pos > 4) {
			std::string last_ext = extract_dir.substr(dot_pos);
			if ((last_ext == ".gz" || last_ext == ".xz" || last_ext == ".bz2" || last_ext == ".Z") &&
				extract_dir.substr(dot_pos - 4, 4) == ".tar") {
				dot_pos -= 4;
			}
		}

		if (dot_pos != std::string::npos)
			extract_dir = extract_dir.substr(0, dot_pos);

		exp->CreateDirectory(extract_dir);

		struct archive *a = archive_read_new();
		archive_read_support_format_all(a);
		archive_read_support_filter_all(a);

		if (archive_read_open_filename(a, archive_path.c_str(), 512 * 1024) != ARCHIVE_OK) { archive_read_free(a); return false; }

		struct archive_entry *entry;

		u64 last_reported_bytes = 0;

		while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
				std::string current_file = extract_dir + "/" + std::string(archive_entry_pathname(entry));

				if (current_file.find("..") != std::string::npos) {
					GLEAF_WARN_FMT("SKIPPED FILE. Unsafe path detected: %s", current_file.c_str());
					continue;
				}
				
				if (archive_entry_filetype(entry) == AE_IFDIR) {
					exp->CreateDirectory(current_file);
					GLEAF_LOG_FMT("Making directory: %s", current_file.c_str());
					continue;
				}

				auto f = fopen(current_file.c_str(), "wb");

				const void* workbuf;
				size_t size;
				u64 offset;
				u64 current_pos = 0;

				char write_buf[256 * 1024];
				setvbuf(f, write_buf, _IOFBF, sizeof(write_buf));
				
				while (archive_read_data_block(a, &workbuf, &size, (int64_t*) &offset) == ARCHIVE_OK) {
					GLEAF_LOG_FMT("Writing file: %s (Offset: %ld, Size: %zu)", current_file.c_str(), offset, size);
					
					// exp->WriteFile(current_file, workbuf, size);
					// TODO: add offset writing function to StdExplorer?

					if((u64)offset != current_pos) {
						fseeko(f, offset, SEEK_SET);
						current_pos = offset;
					}

					current_pos += fwrite(workbuf, 1, size, f);
					
					u64 current_compressed_bytes = archive_filter_bytes(a, -1);
					if(current_compressed_bytes - last_reported_bytes > 128 * 1024) {
						prog_cb(current_compressed_bytes - last_reported_bytes);
						last_reported_bytes = current_compressed_bytes;
					}
				}
				fclose(f);
			}

		if(total_archive_size > last_reported_bytes) {
			prog_cb(total_archive_size - last_reported_bytes);
		}
		archive_read_free(a);
		return true;
	}
}