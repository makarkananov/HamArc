#include <iostream>
#include <string>
#include <vector>

std::vector<bool> encode(const std::string& sent_string) {
  std::vector<bool> result;
  int i = 0;
  int j = 1;
  while (i < sent_string.length()) {
    if (result.size() + 1 == j) {
      result.push_back(false);
      j *= 2;
    } else {
      result.push_back(sent_string[i] == '1');
      i++;
    }
  }
  result.push_back(false);
  int cnt = 0;
  for (int i = 0; i < result.size(); i += 2) cnt ^= result[i];
  result[0] = cnt;
  int control_bit_ind = 2;
  while (control_bit_ind < result.size()) {
    cnt = 0;
    int i = control_bit_ind;
    int j = 1;
    while (i < result.size()) {
      if (j == control_bit_ind) {
        i += control_bit_ind;
        j = 0;
      } else {
        cnt ^= result[i];
        i++;
        j++;
      }
    }
    result[control_bit_ind - 1] = cnt;
    control_bit_ind *= 2;
  }
  cnt = 0;
  for (int i = 0; i < result.size() - 1; i += 1) cnt ^= result[i];
  result[result.size() - 1] = cnt;
  return result;
}
std::vector<bool> decode(const std::string& received_string, const uint32_t& word_length, const uint32_t& file_size) {
  std::vector<bool> check;
  uint32_t bits_per_word = ((log(word_length*CHAR_BIT)/log(2) + 1) + word_length*CHAR_BIT + 1);
  if (received_string.length() < bits_per_word) {
    uint32_t modulo_word_length = received_string.size() / 8;
    uint32_t bits_per_modulo_word = ((log(modulo_word_length*CHAR_BIT)/log(2) + 1) + modulo_word_length*CHAR_BIT + 1);
    while(bits_per_modulo_word >= received_string.size()){
      bits_per_modulo_word = ((log(modulo_word_length*CHAR_BIT)/log(2) + 1) + modulo_word_length*CHAR_BIT + 1);
      modulo_word_length--;
    }
    for (int i = 0; i < bits_per_modulo_word; ++i) check.push_back(received_string[i] == '1');
  } else {
    for (int i = 0; i < bits_per_word; ++i) check.push_back(received_string[i] == '1');
  }
  int cnt = 0;
  for (int i = 2; i < check.size() - 1; i += 2) cnt ^= check[i];
  check[0] = cnt;
  int control_bit_ind = 2;
  while (control_bit_ind < check.size() - 1) {
    cnt = 0;
    uint32_t i = control_bit_ind;
    uint32_t j = 1;
    while (i < check.size() - 1) {
      if (j == control_bit_ind) {
        i += control_bit_ind;
        j = 0;
      } else {
        cnt ^= check[i];
        i++;
        j++;
      }
    }
    check[control_bit_ind - 1] = cnt;
    control_bit_ind *= 2;
  }
  cnt = 0;
  for (int i = 0; i < check.size() - 1; i += 1) cnt ^= check[i];
  check[check.size() - 1] = cnt;
  uint32_t error_sm = 0;
  for (int i = 0; i < check.size() - 1; ++i) {
    if ((check[i] == true) == (received_string[i] == '0')) error_sm += (i + 1);
  }
  if (error_sm != 0 && (check[check.size() - 1] == true) != (received_string[check.size() - 1] == '0')) {
    check[error_sm - 1] = check[error_sm - 1] ^ 1;
    std::cout << "1 error has been corrected!" << std::endl;
  }
  if (error_sm != 0 && (check[check.size() - 1] == true) == (received_string[check.size() - 1] == '0'))
    std::cout << "2 errors have been found and cannot be corrected, extracted file is broken :(" << std::endl;
  std::vector<bool> result;
  for (int i = 0; i < check.size() - 1; ++i) {
    if ((i + 1) & (i)) {
      result.push_back(check[i]);
    }
  }
  return result;
}
