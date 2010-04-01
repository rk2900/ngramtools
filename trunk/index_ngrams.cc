#include <iostream>
#include <string>
#include <vector>

typedef long long int64;

using namespace std;

int main(int argc, char* argv[]) {
  int index_chunk_size = 1000000;
  if (argc < 2) {
    cerr << "USAGE: ./index_ngrams FILE [CHUNK_SIZE] < INPUT > OUTPUT" << endl;
    exit(1);
  } else if (argc > 2) {
    index_chunk_size = atoi(argv[2]);
  }
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
      cout << '\t' << argv[1] << '\t' << index;
    cout << endl;
  }
}
