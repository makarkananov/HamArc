#pragma once

#include <cinttypes>
#include <string>
#include <vector>

struct Options{
  bool append = false;
  bool create = false;
  bool extract = false;
  bool list = false;
  bool del = false;
  bool concatenate = false;
  uint32_t word_length = 1;
  std::vector<std::string> files;
  std::string archive_name;
  std::string conc_archive1;
  std::string conc_archive2;
};

Options parse(const uint32_t& arguments_amount, char* arguments_array[]);
