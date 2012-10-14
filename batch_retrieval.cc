#include <iostream>
#include <fstream>

#include "string_utils.h"
#include "bloom-read-only/bloom_filter.hpp"

using namespace std;

DEFINE_int(predicted_element_count, 10000000, "");
DEFINE_double(false_positive_probability, 0.001, "");
DEFINE_string(phrase_list, "", "");

int main(int argc, char* argv[]) {
  ParseArgs(argc, argv);

  int random_seed = 0;

  if (phrase_list == "") {
    cerr << "USAGE: batch_retrieval --phrase_list FILE" << endl;
    exit(1);
  }
  string line, all;
  ifstream file(phrase_list.c_str());
  bloom_filter bloom(predicted_element_count, false_positive_probability, random_seed);
  while (getline(file, line)) {
    bloom.insert(line);
  }
  while (getline(cin, line)) {
    int p = line.find_first_of('\t');
    if (bloom.contains(line.substr(0, p))) {
      cout << line << endl;
    }
  }
}
