#include <parsing.h>

#include <iostream>
#include <cinttypes>
#include <string>

Options parse(const uint32_t& arguments_amount, char* arguments_array[]) {
  Options OptionsList;
  uint32_t i = 1;
  while (i < arguments_amount) {
    if (arguments_array[i][0] == '-') {
      if (strcmp(arguments_array[i], "--create") == 0 || strcmp(arguments_array[i], "-c") == 0) {
        OptionsList.create = true;
      } else if (strcmp(arguments_array[i], "--word-length") == 0 || strcmp(arguments_array[i], "-w") == 0) {
        i++;
        if(i < arguments_amount) OptionsList.word_length = atoi(arguments_array[i]);
      }else if (strcmp(arguments_array[i], "--list") == 0 || strcmp(arguments_array[i], "-l") == 0) {
        OptionsList.list = true;
      } else if (strcmp(arguments_array[i], "--extract") == 0 || strcmp(arguments_array[i], "-x") == 0) {
        OptionsList.extract = true;
      } else if (strcmp(arguments_array[i], "--append") == 0 || strcmp(arguments_array[i], "-a") == 0) {
        OptionsList.append = true;
      } else if (strcmp(arguments_array[i], "--delete") == 0 || strcmp(arguments_array[i], "-d") == 0) {
        OptionsList.del = true;
      } else if (strcmp(arguments_array[i], "--concatenate") == 0 || strcmp(arguments_array[i], "-A") == 0) {
        OptionsList.concatenate = true;
        if (i + 2 < arguments_amount) {
          i++;
          OptionsList.conc_archive1 = arguments_array[i];
          i++;
          OptionsList.conc_archive2 = arguments_array[i];
        }
      } else if (strcmp(arguments_array[i], "-f") == 0) {
        i++;
        if (i < arguments_amount) {
          OptionsList.archive_name = arguments_array[i];
        }
      } else if (std::string(arguments_array[i]).substr(0, 6).compare("--file") == 0) {
        OptionsList.archive_name = std::string(arguments_array[i]).substr(7);
      }
    } else {
      OptionsList.files.push_back(arguments_array[i]);
    }
    i++;
  }

  return OptionsList;
}