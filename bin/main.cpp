#include "parsing.h"
#include "archive.h"

#include <iostream>

int main(int argc, char* argv[]){
  Options NowOptions = parse(argc, argv);

  if(NowOptions.create){
    Archive MainArchive(NowOptions.archive_name, NowOptions.files, NowOptions.word_length);
  } else if(NowOptions.append){
    Archive MainArchive = readArchive(NowOptions.archive_name);
    MainArchive.append(NowOptions.files);
  } else if(NowOptions.del){
    Archive MainArchive = readArchive(NowOptions.archive_name);
    MainArchive.del(NowOptions.files);
  } else if(NowOptions.concatenate){
    Archive MainArchive = readArchive(NowOptions.conc_archive1);
    MainArchive.concatenate(NowOptions.conc_archive2, NowOptions.archive_name);
  } else if(NowOptions.list){
    Archive MainArchive = readArchive(NowOptions.archive_name);
    MainArchive.list();
  } else if(NowOptions.extract){
    Archive MainArchive = readArchive(NowOptions.archive_name);
    MainArchive.extract(NowOptions.files);
  }
  return 0;
}