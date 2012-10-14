/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include <iostream>
#include <fstream>
#include <cassert>
#include <map>
#include "string_utils.h"

DEFINE_string(bounds, "./index.txt", "");
DEFINE_string(prefix, "ngm", "");
DEFINE_string(stop_words, "./stop_words.txt", "");
DEFINE_int(min_count, 40, "");

class NgramRotater {
  map<string, int> index_;
  vector<ofstream*> files_;
  vector<string> cache_;
  hash_set<string, StringHash>* stopwords_;
  int count_;
 public:
  NgramRotater() {
    count_ = 0;
    stopwords_ = 0;
  }

  hash_set<string, StringHash>* GetStopWords() {
    if (stopwords_ == 0) {
      stopwords_ = new hash_set<string, StringHash>;
      if (stop_words != "") {
	ifstream input(stop_words.c_str());
	string line;
	while (getline(input, line)) {
	  vector<string> fields;
	  SplitStringUsing(line, "\t\n", &fields);
	  if (!fields.empty() > 0)
	    stopwords_->insert(fields[0]);
	}
      }
    }
    return stopwords_;
  }

  void FlushCache() {
    for (int i = 0; i < files_.size(); ++i) {
      if (!cache_[i].empty()) {
        *files_[i] << cache_[i];
        cache_[i].clear();
      }
    }
  }

  void RotateOne(const vector<string>& fields, int pos, const string& value) {
    int len = fields.size();
    string s;
    if (fields.size() == 0)
      return;
    hash_set<string, StringHash>* stopwords = GetStopWords();
    if (stopwords->find(fields[pos]) != stopwords->end()) {
      if (pos > 0) return;
      int64 c = atol(value.c_str());
      if (c < 10 * min_count)
        return;
    }
    for (int i = pos; i < len; ++i) {
      if (!s.empty())
        s += ' ';
      s += fields[i];
    }
    if (pos > 0)
      s += " ><";
    for (int i = pos - 1; i >= 0; --i) {
      if (!s.empty())
        s += ' ';
      s += fields[i];
    }
    s += "\t";
    s += value;
    s += "\n";
    map<string, int>::const_iterator b = index_.upper_bound(fields[pos]);
    int findex = files_.size() - 1;
    if (b != index_.end())
      findex = b->second;
    cache_[findex] += s;
  }
 public:
  void Rotate(const char* index_file_name) {
    ifstream index_file(index_file_name);
    string line;
    int count = 0;
    while (getline(index_file, line)) {
      int tab = line.find_first_of('\t');
      string key;
      if (tab != string::npos)
	key = line;
      else
	key = line.substr(tab + 1);
      index_[key] = count++;
    }
    count += 2;
    files_.resize(count);
    cache_.resize(count);
    for (int i = 0; i < count; ++i) {
      string name = StringPrintf("%s.%03d", prefix.c_str(), i);
      files_[i] = new ofstream(name.c_str());
      cache_[i].reserve(200000);
    }
    count_ = 0;
    while (getline(cin, line)) {
      for (int i = 0; i < line.size(); ++i) {
        if (isdigit(line[i]))
          line[i] = '0';
        else if (line[i] == '\t')
          break;
      }
      int tab = line.find_first_of('\t');
      assert(tab != string::npos);
      vector<string> fields;
      SplitStringUsing(line.substr(0, tab), " ", &fields);
      string value = line.substr(tab + 1);
      for (int i = 0; i < fields.size(); ++i) {
        RotateOne(fields, i, value);
      }
      if (++count_ > 100000)
        FlushCache();
    }
    FlushCache();
    for (int i = 0; i < files_.size(); ++i) {
      files_[i]->close();
      delete files_[i];
    }
  }
};

int main(int argc, char* argv[]) {
  ParseArgs(argc, argv);
  NgramRotater r;
  r.Rotate(bounds.c_str());
}
