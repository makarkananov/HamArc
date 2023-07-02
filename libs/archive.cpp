#include <archive.h>
#include <hamming.h>

#include <bitset>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

Archive::Archive(const std::string& archive_name_arg) {
  archive_name = archive_name_arg;
}
Archive::Archive(const std::string& archive_name_arg, std::vector<std::string>& files_arg, const uint32_t& word_length_arg) {
  archive_name = archive_name_arg;
  files = files_arg;
  word_length = word_length_arg;
  files_number = files.size();

  std::ofstream arc_file(archive_name, std::ios_base::binary);
  if (!arc_file.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }
  arc_file.write(reinterpret_cast<const char*>(&files_number), Archive::kFilesNumberBytes);
  arc_file.write(reinterpret_cast<const char*>(&word_length), Archive::kWordLengthBytes);
  for (int i = 0; i < files_number; ++i) {
    uint32_t n = files[i].size();
    arc_file.write((const char*)&n, Archive::kFileNameLengthBytes);
    arc_file.write(files[i].c_str(), n);
  }
  for (int i = 0; i < files_number; ++i) {
    std::ifstream file(files[i], std::ios_base::binary);
    if (!file.is_open()) {
      std::cout << "Error opening file!" << std::endl;
      exit(1);
    }
    file.seekg(0, std::ios::end);
    uint32_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    uint32_t bits_per_word = (log(word_length*CHAR_BIT)/log(2) + 1) + word_length*CHAR_BIT + 1;
    uint32_t bits_per_modulo_word = (log((file_size%word_length)*CHAR_BIT)/log(2) + 1) + (file_size%word_length)*CHAR_BIT + 1;
    if (file_size%word_length != 0) {;
      file_size = (file_size/word_length)*((bits_per_word + (CHAR_BIT - bits_per_word%CHAR_BIT))/CHAR_BIT)
          + ((bits_per_modulo_word + (CHAR_BIT - bits_per_modulo_word%CHAR_BIT))/CHAR_BIT);
    } else {
      file_size = (file_size/word_length)*((bits_per_word + (CHAR_BIT - bits_per_word%CHAR_BIT))/CHAR_BIT);
    }
    arc_file.write((const char*)&file_size, Archive::kFileLengthBytes);
    std::bitset<CHAR_BIT> bits;
    char ch;
    std::string word = "";
    uint32_t j = 1;
    while (file.get(ch)) {
      bits = ch;
      word += bits.to_string();
      if (j%word_length == 0) {
        std::vector<bool> encoded_word = encode(word);
        writeBits(encoded_word, arc_file);
        word = "";
      }
      j++;
    }
    if (word != "" && word_length != 1) {
      std::vector<bool> encoded_word = encode(word);
      writeBits(encoded_word, arc_file);
    }
  }
};

void Archive::writeBits(std::vector<bool>& bits_to_write, std::ofstream& fout) {
  int k = 0;
  while (k < bits_to_write.size()) {
    std::bitset<CHAR_BIT> byte_to_write;
    for (int l = k; l - k < CHAR_BIT && l < bits_to_write.size(); ++l) {
      byte_to_write.set(CHAR_BIT - (l - k) - 1, bits_to_write[l]);
    }
    unsigned long n = byte_to_write.to_ulong();
    fout.write(reinterpret_cast<const char*>(&n), 1);
    k += CHAR_BIT;
  }
}

Archive readArchive(const std::string& archive_name_arg) {
  Archive ReadArchive(archive_name_arg);
  ReadArchive.archive_name = archive_name_arg;
  std::ifstream file(ReadArchive.archive_name, std::ios_base::binary);
  if (!file.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }

  file.read((char*)&ReadArchive.files_number, sizeof(ReadArchive.files_number));
  file.read((char*)&ReadArchive.word_length, sizeof(ReadArchive.word_length));
  for (int i = 0; i < ReadArchive.files_number; ++i) {
    uint32_t n;
    file.read((char*)&n, sizeof(n));
    std::string name(n, 0);

    for (int i = 0; i < n; ++i) file.read(&name[i], sizeof(name[0]));
    ReadArchive.files.push_back(name);
  }
  for (int i = 0; i < ReadArchive.files_number; ++i) {
    std::bitset<CHAR_BIT> bits;
    char ch;
    uint32_t file_size;
    file.read((char*)&file_size, sizeof(file_size));
    for (int j = 0; j < file_size; ++j) {
      file.get(ch);
      bits = ch;
    }
  }
  return ReadArchive;
}

void Archive::list() {
  for (int i = 0; i < files_number; ++i) {
    std::cout << files[i] << " ";
  }
  std::cout << std::endl;
}

void Archive::extract(const std::vector<std::string>& extract_files) {
  std::set<uint32_t> extract_positions;
  for (int i = 0; i < extract_files.size(); ++i) {
    for (int j = 0; j < files_number; ++j) {
      if (files[j] == extract_files[i]) {
        extract_positions.insert(j);
        break;
      }
    }
  }
  std::ifstream fin(archive_name, std::ios_base::binary);
  if (!fin.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }
  std::ofstream fout(archive_name + "_tmp", std::ios_base::binary);
  if (!fout.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }

  uint32_t n;
  fin.read((char*)&n, Archive::kFilesNumberBytes);
  n -= extract_positions.size();
  fout.write(reinterpret_cast<const char*>(&n), Archive::kFilesNumberBytes);
  fin.read((char*)&n, Archive::kWordLengthBytes);
  fout.write(reinterpret_cast<const char*>(&n), Archive::kWordLengthBytes);

  for (int i = 0; i < files_number; ++i) {
    if (extract_positions.count(i)) {
      fin.read((char*)&n, Archive::kFileNameLengthBytes);
      fin.seekg(n, std::ios::cur);
    } else {
      fin.read((char*)&n, Archive::kFileNameLengthBytes);
      fout.write(reinterpret_cast<const char*>(&n), Archive::kFileNameLengthBytes);
      for (int j = 0; j < n; ++j) {
        char ch;
        fin.read((char*)&ch, 1);
        fout.write(reinterpret_cast<const char*>(&ch), 1);
      }
    }
  }
  for (int i = 0; i < files_number; ++i) {
    if (extract_positions.count(i)) {
      std::ofstream extracted_fout(files[i], std::ios_base::binary);
      if (!extracted_fout.is_open()) {
        std::cout << "Error opening file!" << std::endl;
        exit(1);
      }
      fin.read((char*)&n, Archive::kFileLengthBytes);
      std::string word = "";
      char ch;
      uint32_t bits_per_word = ((log(word_length*CHAR_BIT)/log(2) + 1) + word_length*CHAR_BIT + 1);
      uint32_t bytes_per_word = (bits_per_word + (CHAR_BIT - bits_per_word % CHAR_BIT)) / CHAR_BIT;
      for (int j = 0; j < n; ++j) {
        fin.read((char*)&ch, 1);
        std::bitset<CHAR_BIT> bits = ch;
        word += bits.to_string();
        if ((j + 1)%bytes_per_word == 0 && j != 0) {
          std::vector<bool> decoded_word = decode(word, word_length, n);
          writeBits(decoded_word, extracted_fout);
          word = "";
        }
      }
      if (word != "" && word_length != 1) {
        std::vector<bool> decoded_word = decode(word, word_length, n);
        writeBits(decoded_word, extracted_fout);
      }
    } else {
      fin.read((char*)&n, Archive::kFileNameLengthBytes);
      fout.write(reinterpret_cast<const char*>(&n), Archive::kFileNameLengthBytes);
      for (int j = 0; j < n; ++j) {
        char ch;
        fin.read((char*)&ch, 1);
        fout.write(reinterpret_cast<const char*>(&ch), 1);
      }
    }
  }

  fin.close();
  fout.close();
  std::remove(archive_name.c_str());
  std::rename((archive_name + "_tmp").c_str(), archive_name.c_str());
}

void Archive::del(const std::vector<std::string>& delete_files) {
  std::set<int> delete_positions;
  for (int i = 0; i < delete_files.size(); ++i) {
    for (int j = 0; j < files_number; ++j) {
      if (files[j] == delete_files[i]) {
        delete_positions.insert(j);
        break;
      }
    }
  }
  std::ifstream fin(archive_name, std::ios_base::binary);
  if (!fin.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }
  std::ofstream fout(archive_name + "_tmp", std::ios_base::binary);
  if (!fout.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }
  uint32_t n;
  fin.read((char*)&n, Archive::kFilesNumberBytes);
  n -= delete_positions.size();
  fout.write(reinterpret_cast<const char*>(&n), Archive::kFilesNumberBytes);
  fin.read((char*)&n, Archive::kWordLengthBytes);
  fout.write(reinterpret_cast<const char*>(&n), Archive::kWordLengthBytes);

  for (int i = 0; i < files_number; ++i) {
    if (delete_positions.count(i)) {
      fin.read((char*)&n, Archive::kFileNameLengthBytes);
      fin.seekg(n, std::ios::cur);
    } else {
      fin.read((char*)&n, Archive::kFileNameLengthBytes);
      fout.write(reinterpret_cast<const char*>(&n), Archive::kFileNameLengthBytes);
      for (int j = 0; j < n; ++j) {
        char ch;
        fin.read((char*)&ch, 1);
        fout.write(reinterpret_cast<const char*>(&ch), 1);
      }
    }
  }
  for (int i = 0; i < files_number; ++i) {
    if (delete_positions.count(i)) {
      fin.read((char*)&n, Archive::kFileNameLengthBytes);
      fin.seekg(n, std::ios::cur);
    } else {
      fin.read((char*)&n, Archive::kFileNameLengthBytes);
      fout.write(reinterpret_cast<const char*>(&n), Archive::kFileNameLengthBytes);
      for (int j = 0; j < n; ++j) {
        char ch;
        fin.read((char*)&ch, 1);
        fout.write(reinterpret_cast<const char*>(&ch), 1);
      }
    }
  }
  fin.close();
  fout.close();

  std::remove(archive_name.c_str());
  std::rename((archive_name + "_tmp").c_str(), archive_name.c_str());
}

void Archive::append(const std::vector<std::string>& append_files) {
  std::ifstream fin(archive_name, std::ios_base::binary);
  if (!fin.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }
  std::ofstream fout(archive_name + "_tmp", std::ios_base::binary);
  if (!fout.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }
  uint32_t n;
  fin.read((char*)&n, Archive::kFilesNumberBytes);
  n += append_files.size();
  fout.write(reinterpret_cast<const char*>(&n), Archive::kFilesNumberBytes);
  fin.read((char*)&n, Archive::kWordLengthBytes);
  fout.write(reinterpret_cast<const char*>(&n), Archive::kWordLengthBytes);

  for (int i = 0; i < files_number; ++i) {
    fin.read((char*)&n, Archive::kFileNameLengthBytes);
    fout.write(reinterpret_cast<const char*>(&n), Archive::kFileNameLengthBytes);
    for (int j = 0; j < n; ++j) {
      char ch;
      fin.read((char*)&ch, 1);
      fout.write(reinterpret_cast<const char*>(&ch), 1);
    }
  }
  for (int i = 0; i < append_files.size(); ++i) {
    n = append_files[i].size();
    fout.write((const char*)&n, sizeof(n));
    fout.write(append_files[i].c_str(), n);
  }
  for (int i = 0; i < files_number; ++i) {
    fin.read((char*)&n, Archive::kFileLengthBytes);
    fout.write(reinterpret_cast<const char*>(&n), Archive::kFileLengthBytes);
    for (int j = 0; j < n; ++j) {
      char ch;
      fin.read((char*)&ch, 1);
      fout.write(reinterpret_cast<const char*>(&ch), 1);
    }
  }
  for (int i = 0; i < append_files.size(); ++i) {
    std::ifstream append_fin(append_files[i], std::ios_base::binary);
    if (!append_fin.is_open()) {
      std::cout << "Error opening file!" << std::endl;
      exit(1);
    }
    append_fin.seekg(0, std::ios::end);
    uint32_t file_size = append_fin.tellg();
    append_fin.seekg(0, std::ios::beg);
    int bits_per_word = (log(word_length*CHAR_BIT)/log(2) + 1) + word_length*CHAR_BIT;
    if (file_size%word_length != 0) {
      uint32_t bits_per_modulo_word = (log((file_size%word_length)*CHAR_BIT)/log(2) + 1) + (file_size%word_length)*CHAR_BIT;
      file_size = (file_size/word_length)*((bits_per_word + (CHAR_BIT - bits_per_word%CHAR_BIT))/CHAR_BIT)
          + ((bits_per_modulo_word + (CHAR_BIT - bits_per_modulo_word%CHAR_BIT))/CHAR_BIT);
    } else {
      file_size = (file_size/word_length)*((bits_per_word + (CHAR_BIT - bits_per_word%CHAR_BIT))/CHAR_BIT);
    }
    fout.write((const char*)&file_size, Archive::kFileLengthBytes);
    if (!append_fin.is_open()) {
      std::cout << "Error opening file!" << std::endl;
      exit(1);
    }
    std::bitset<CHAR_BIT> bits;
    char ch;
    std::string word = "";
    uint32_t j = 1;
    while (append_fin.get(ch)) {
      bits = ch;
      word += bits.to_string();
      if (j%word_length == 0) {
        std::vector<bool> encoded_word = encode(word);
        writeBits(encoded_word, fout);
        word = "";
      }
      j++;
    }
    if (word != "" && word_length != 1) {
      std::vector<bool> encoded_word = encode(word);
      writeBits(encoded_word, fout);
    }
  }
  fin.close();
  fout.close();
  std::remove(archive_name.c_str());
  std::rename((archive_name + "_tmp").c_str(), archive_name.c_str());
}

void Archive::concatenate(const std::string& archive_to_conc, const std::string& new_archive) {
  std::ifstream main_fin(archive_name, std::ios_base::binary);
  if (!main_fin.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }
  std::ifstream to_conc_fin(archive_to_conc, std::ios_base::binary);
  if (!to_conc_fin.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }
  std::ofstream fout(new_archive, std::ios_base::binary);
  if (!fout.is_open()) {
    std::cout << "Error opening file!" << std::endl;
    exit(1);
  }
  uint32_t n;
  uint32_t to_conc_files_number;
  main_fin.read((char*)&n, Archive::kFilesNumberBytes);
  to_conc_fin.read((char*)&to_conc_files_number, Archive::kFilesNumberBytes);
  n += to_conc_files_number;
  fout.write(reinterpret_cast<const char*>(&n), Archive::kFilesNumberBytes);
  main_fin.read((char*)&n, Archive::kWordLengthBytes);
  to_conc_fin.seekg(Archive::kWordLengthBytes, std::ios::cur);
  fout.write(reinterpret_cast<const char*>(&n), Archive::kWordLengthBytes);

  for (int i = 0; i < files_number; ++i) {
    main_fin.read((char*)&n, Archive::kFileNameLengthBytes);
    fout.write(reinterpret_cast<const char*>(&n), Archive::kFileNameLengthBytes);
    for (int j = 0; j < n; ++j) {
      char ch;
      main_fin.read((char*)&ch, 1);
      fout.write(reinterpret_cast<const char*>(&ch), 1);
    }
  }
  for (int i = 0; i < to_conc_files_number; ++i) {
    to_conc_fin.read((char*)&n, Archive::kFileNameLengthBytes);
    fout.write(reinterpret_cast<const char*>(&n), Archive::kFileNameLengthBytes);
    for (int j = 0; j < n; ++j) {
      char ch;
      to_conc_fin.read((char*)&ch, 1);
      fout.write(reinterpret_cast<const char*>(&ch), 1);
    }
  }
  for (int i = 0; i < files_number; ++i) {
    main_fin.read((char*)&n, Archive::kFileLengthBytes);
    fout.write(reinterpret_cast<const char*>(&n), Archive::kFileLengthBytes);
    for (int j = 0; j < n; ++j) {
      char ch;
      main_fin.read((char*)&ch, 1);
      fout.write(reinterpret_cast<const char*>(&ch), 1);
    }
  }
  for (int i = 0; i < to_conc_files_number; ++i) {
    to_conc_fin.read((char*)&n, Archive::kFileLengthBytes);
    fout.write(reinterpret_cast<const char*>(&n), Archive::kFileLengthBytes);
    for (int j = 0; j < n; ++j) {
      char ch;
      to_conc_fin.read((char*)&ch, 1);
      fout.write(reinterpret_cast<const char*>(&ch), 1);
    }
  }

  main_fin.close();
  to_conc_fin.close();
  fout.close();
}