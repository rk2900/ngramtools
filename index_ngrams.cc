#include <iostream>
#include <string>
#include <vector>

#include "string_utils.h"

DEFINE_string(index_file, "", "FILE.index");
DEFINE_string(stop_words, "./stop_words.txt", "");

DEFINE_int(index_chunk_size, 10000000,
	   "Each chunk has an entry in the index. The ngram search program first use binary search to identify the chunk that contains the ngram, and then does a linear scan within the chunk. The smaller chunk size reduces the amount of linear scan, but will result in larger index.");

typedef long long int64;

using namespace std;

int main(int argc, char* argv[]) {
  ParseArgs(argc, argv);
  int64 index = 0, bound = 0;
  string input;
  vector<pair<string, int64> > indices;
  string line, key;
  while (getline(cin, input)) {
    int p = input.find_first_of('\t');
    key = input.substr(0, p);
    line.clear();
    bool new_chunk = false;
    if (index >= bound) {
      indices.push_back(make_pair(key, index));
      bound += index_chunk_size;
      new_chunk = true;
    }
    line = input;
    if (!line.empty()) {
      index += line.size() + 1;
    }
  }
  if (indices.empty() && !key.empty())
    indices.push_back(make_pair(key, 0));
  for (int i = 0; i < indices.size(); ++i) {
    cout << indices[i].first << '\t' << indices[i].second;
    if (i == 0)
      cout << '\t' << index_file << '\t' << index;
    cout << endl;
  }
}
