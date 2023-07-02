#include <string>
#include <vector>

std::vector<bool> encode(const std::string& sent_string);
std::vector<bool> decode(const std::string& received_string, const uint32_t& word_length, const uint32_t& file_size);