#include <fstream>
#include <string>
#include <vector>

class Archive{
 public:
  const uint8_t kFilesNumberBytes = 4;
  const uint8_t kWordLengthBytes = 4;
  const uint8_t kFileNameLengthBytes = 4;
  const uint8_t kFileLengthBytes = 4;
  std::string archive_name;
  std::vector<std::string> files;
  uint32_t files_number;
  uint32_t word_length;
  Archive(const std::string& archive_name_arg, std::vector<std::string>& files_arg, const uint32_t& word_length_arg);
  Archive(const std::string& archive_name_arg);
  void list();
  void extract(const std::vector<std::string>& extract_files);
  void del(const std::vector<std::string>& delete_files);
  void append(const std::vector<std::string>& append_files);
  void concatenate(const std::string& archive_to_conc, const std::string& new_archive);

 private:
  void writeBits(std::vector<bool>& bits_to_write, std::ofstream& fout);
};

Archive readArchive(const std::string& archive_name_arg);