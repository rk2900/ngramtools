#include <iostream>
#include <fstream>

#include "npattern.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cerr << "USAGE: batch_counting PATTERN" << endl;
    exit(1);
  }
  string line, all;
  const char* args = argv[1];
  if (argc > 2 && (string("-file") == argv[1] || string("-f") == argv[1])) {
    ifstream file(argv[2]);
    while (getline(file, line)) {
      all += line + "\n";
    }
    args = all.c_str();
  }
  NgramInfoCollector* col = NgramInfoCollector::Create(args);
  Ngram ngram;
  col->Start();
  while (getline(cin, line)) {
    ngram.Init(line);
    col->CollectInfo(ngram);
  }
  col->Flush();
}
