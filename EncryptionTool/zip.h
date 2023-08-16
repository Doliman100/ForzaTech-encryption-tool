#pragma once

#include <array>
#include <vector>
#include <map>
#include <functional>

namespace Zip {
  class Exception : public std::exception {};

  class Header {
  public:
    virtual ~Header() {};

    virtual void Write(std::ostream &os) = 0;
  };

  class LocalFileHeader : public Header {
  public:
    LocalFileHeader(std::istream &input);

    void Write(std::ostream &os) override;

    uint16_t compression_method() { return compression_method_; }
    void compression_method(uint16_t value) { compression_method_ = value; }
    uint32_t compressed_size() { return compressed_size_; }
    void compressed_size(uint32_t value) { compressed_size_ = value; }
    std::string &file_name() { return file_name_; }
    uint32_t offset_begin() { return offset_begin_; }
    uint32_t offset_end() { return offset_end_; }

    static const uint32_t signature = 0x04034B50;

  private:
    uint16_t version_extract_;
    uint16_t general_flag_;
    uint16_t compression_method_;
    uint16_t time_;
    uint16_t date_;
    uint32_t crc32_;
    uint32_t compressed_size_;
    uint32_t uncompressed_size_;
    std::string file_name_;
    std::vector<uint8_t> extra_field_;

    uint32_t offset_begin_;
    uint32_t offset_end_;
  };

  class CentralDirectoryHeader : public Header {
  public:
    CentralDirectoryHeader(std::istream &input);

    void Write(std::ostream &os) override;

    void compression_method(uint16_t value) { compression_method_ = value; }
    void compressed_size(uint32_t value) { compressed_size_ = value; }
    uint32_t offset() { return offset_; }
    void offset(uint32_t value) { offset_ = value; }
    std::vector<uint8_t> &extra_field() { return extra_field_; }

    static const uint32_t signature = 0x02014B50;

  private:
    uint16_t version_made_;
    uint16_t version_extract_;
    uint16_t general_flag_;
    uint16_t compression_method_;
    uint16_t time_;
    uint16_t date_;
    uint32_t crc32_;
    uint32_t compressed_size_;
    uint32_t uncompressed_size_;
    uint16_t disk_id_;
    uint16_t internal_file_attributes_;
    uint32_t external_file_attributes_;
    uint32_t offset_;
    std::string file_name_;
    std::vector<uint8_t> extra_field_;
    std::string comment_;
  };

  class EndOfCentralDirectoryRecord : public Header {
  public:
    EndOfCentralDirectoryRecord(std::istream &input);

    void Write(std::ostream &os) override;

    void cd_offset(uint32_t value) { cd_offset_ = value; }

    static const uint32_t signature = 0x06054B50;

  private:
    uint16_t disk_id_;
    uint16_t cd_disk_;
    uint16_t cd_entries_;
    uint16_t total_cd_entries_;
    uint32_t cd_size_;
    uint32_t cd_offset_;
    std::string comment_;
  };

  class Transfer {
  public:
    Transfer(std::istream &input, std::ostream &output);

    void Run(std::function<bool(LocalFileHeader &)> callback);

    operator bool() {
      return is_open_;
    }

  private:
    bool is_open_ = false;

    std::istream &is_;
    std::ostream &os_;

    std::map<uint32_t, LocalFileHeader> local_file_headers_;
  };
}
