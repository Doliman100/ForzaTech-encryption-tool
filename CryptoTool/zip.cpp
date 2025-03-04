#include <iostream>
#include "iobinary.h"
#include "zip.h"

// https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT

// extra fields
// u16 0x1123
//   u16 0x0004
//   u32 offset to the end of local_file_header (begin of data)

Zip::LocalFileHeader::LocalFileHeader(std::istream &is) {
	std::istreambuf_iterator<char> it(is);
	version_extract_ = Read16LE(it);
	general_flag_ = Read16LE(it);
	if (general_flag_ != 0) {
		throw std::runtime_error("General purpose bit flag is not supported.");
	}
	compression_method_ = Read16LE(it);
	time_ = Read16LE(it);
	date_ = Read16LE(it);
	crc32_ = Read32LE(it);
	compressed_size_ = Read32LE(it);
	uncompressed_size_ = Read32LE(it);
	file_name_.resize(Read16LE(it));
	extra_field_.resize(Read16LE(it));
	is.read(reinterpret_cast<char *>(file_name_.data()), file_name_.size());
	is.read(reinterpret_cast<char *>(extra_field_.data()), extra_field_.size());
	if (extra_field_.size() && Read16LE(std::begin(extra_field_)) != 0x1123) {
		throw std::runtime_error("Extra field is not supported.");
	}
}

void Zip::LocalFileHeader::Write(std::ostream &os) {
	offset_begin_ = static_cast<uint32_t>(os.tellp());

	std::ostreambuf_iterator<char> it(os);
	Write32LE(signature, it);
	Write16LE(version_extract_, it);
	Write16LE(general_flag_, it);
	Write16LE(compression_method_, it);
	Write16LE(time_, it);
	Write16LE(date_, it);
	Write32LE(crc32_, it);
	Write32LE(compressed_size_, it);
	Write32LE(uncompressed_size_, it);
	Write16LE(file_name_.size(), it);
	Write16LE(extra_field_.size(), it);
	os.write(reinterpret_cast<char *>(file_name_.data()), file_name_.size());
	os.write(reinterpret_cast<char *>(extra_field_.data()), extra_field_.size());

	offset_end_ = static_cast<uint32_t>(os.tellp());
}

Zip::CentralDirectoryHeader::CentralDirectoryHeader(std::istream &is) {
	std::istreambuf_iterator<char> it(is);
	version_made_ = Read16LE(it);
	version_extract_ = Read16LE(it);
	general_flag_ = Read16LE(it);
	compression_method_ = Read16LE(it);
	time_ = Read16LE(it);
	date_ = Read16LE(it);
	crc32_ = Read32LE(it);
	compressed_size_ = Read32LE(it);
	uncompressed_size_ = Read32LE(it);
	file_name_.resize(Read16LE(it));
	extra_field_.resize(Read16LE(it));
	comment_.resize(Read16LE(it));
	disk_id_ = Read16LE(it);
	internal_file_attributes_ = Read16LE(it);
	external_file_attributes_ = Read32LE(it);
	offset_ = Read32LE(it);
	is.read(file_name_.data(), file_name_.size());
	is.read(reinterpret_cast<char *>(extra_field_.data()), extra_field_.size());
	is.read(comment_.data(), comment_.size());
}

void Zip::CentralDirectoryHeader::Write(std::ostream &os) {
	std::ostreambuf_iterator<char> it(os);
	Write32LE(signature, it);
	Write16LE(version_made_, it);
	Write16LE(version_extract_, it);
	Write16LE(general_flag_, it);
	Write16LE(compression_method_, it);
	Write16LE(time_, it);
	Write16LE(date_, it);
	Write32LE(crc32_, it);
	Write32LE(compressed_size_, it);
	Write32LE(uncompressed_size_, it);
	Write16LE(file_name_.size(), it);
	Write16LE(extra_field_.size(), it);
	Write16LE(comment_.size(), it);
	Write16LE(disk_id_, it);
	Write16LE(internal_file_attributes_, it);
	Write32LE(external_file_attributes_, it);
	Write32LE(offset_, it);
	os.write(file_name_.data(), file_name_.size());
	os.write(reinterpret_cast<char *>(extra_field_.data()), extra_field_.size());
	os.write(comment_.data(), comment_.size());
}

Zip::EndOfCentralDirectoryRecord::EndOfCentralDirectoryRecord(std::istream &is) {
	std::istreambuf_iterator<char> it(is);
	disk_id_ = Read16LE(it);
	cd_disk_ = Read16LE(it);
	cd_entries_ = Read16LE(it);
	total_cd_entries_ = Read16LE(it);
	cd_size_ = Read32LE(it);
	cd_offset_ = Read32LE(it);
	comment_.resize(Read16LE(it));
	is.read(comment_.data(), comment_.size());
}

void Zip::EndOfCentralDirectoryRecord::Write(std::ostream &os) {
	std::ostreambuf_iterator<char> it(os);
	Write32LE(signature, it);
	Write16LE(disk_id_, it);
	Write16LE(cd_disk_, it);
	Write16LE(cd_entries_, it);
	Write16LE(total_cd_entries_, it);
	Write32LE(cd_size_, it);
	Write32LE(cd_offset_, it);
	Write16LE(comment_.size(), it);
	os.write(comment_.data(), comment_.size());
}

Zip::Transfer::Transfer(std::istream &is, std::ostream &os)
	: is_(is)
	, os_(os) {
}

void Zip::Transfer::Run(std::function<bool(LocalFileHeader &)> callback) {
	int64_t central_directory_offset = -1;
	while (!is_.eof()) {
		uint32_t offset = static_cast<uint32_t>(is_.tellg());
		uint32_t signature = Read32LE(std::istreambuf_iterator<char>(is_));
		switch (signature) {
		case Zip::LocalFileHeader::signature: {
			auto &header = local_file_headers_.emplace(offset, is_).first->second;
			if (header.compressed_size() == 0 || !callback(header)) {
				header.Write(os_);
				if (header.compressed_size() != 0) {
					std::copy_n(std::istreambuf_iterator<char>(is_), header.compressed_size(), std::ostreambuf_iterator<char>(os_));
					is_.get(); // copy_n increments input iterator by N-1
				}
			}
			break;
		}
		case Zip::CentralDirectoryHeader::signature: {
			if (central_directory_offset == -1) {
				central_directory_offset = os_.tellp();
			}
			Zip::CentralDirectoryHeader header(is_);
			auto &local_file_header = local_file_headers_.at(header.offset());
			header.compression_method(local_file_header.compression_method());
			header.compressed_size(local_file_header.compressed_size());
			header.offset(local_file_header.offset_begin());
			
			auto it = std::begin(header.extra_field());
			while (it != std::end(header.extra_field())) {
				uint16_t signature = Read16LE(it);
				uint16_t size = Read16LE(it + 2);
				if (signature == 0x1123 && size == 0x0004) {
					Write32LE(local_file_header.offset_end(), it + 4);
				}
				std::advance(it, 4 + size);
			}

			header.Write(os_);
			break;
		}
		case Zip::EndOfCentralDirectoryRecord::signature: {
			Zip::EndOfCentralDirectoryRecord header(is_);
			header.cd_offset(static_cast<uint32_t>(central_directory_offset));
			header.Write(os_);
			break;
		}
		default:
			throw Zip::Exception();
			break;
		}

		// skip zeros
		while (is_.peek() == '\0') {
			is_.get();
		}
	}
}
