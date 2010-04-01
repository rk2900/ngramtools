/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include <iostream>
#include <fstream>
#include <map>
#include <assert.h>
#include <zlib.h>

#include "ngram_iterator.h"
#include "string_utils.h"
#include "vlist.h"
#include "npattern.h"

typedef long long int64;  // using int64 for index just in case some
                          // files are bigger than 4GB

class NgramIterator::Impl {
  struct FileInfo {
    string name;
    fstream* strm;
    int64 size;

    FileInfo() { strm = 0; size = 0;}
    FileInfo(const string& n, int64 s) { name = n; strm = 0; size = s;}
  };

  // The files containing the rotated ngrams
  vector<FileInfo> files_;

  // The directory containing the ngram files.
  string dir_;

  // If index_[key] == (a, b), we should start looking for key in
  // files_[a] starting at position b.
  map<string, pair<int, int64> > index_;

  // If key is greater than the largest element in index_, this is
  // where we should look for it.
  pair<int, int64> last_;

  // A indices (in the files_ vector) of files in which the current
  // key may be located.
  vector<int> locations_;

  int file_idx_;  // the position of the current file in files_
  int64 position_;  // the current position in the current file

  string key_;  // the current key

  // Return first file in locations_ and seek to the start position
  int GetFile(int64 start) {
    if (locations_.empty())
      return -1;
    int f = locations_[0];
    locations_.erase(locations_.begin());
    if (f < 0)
      return -1;
    else if (files_[f].strm == 0) {
      string fname = files_[f].name;
      files_[f].strm = new fstream(fname.c_str(), ios::in | ios::binary);
    }
    files_[f].strm->seekg(start, ios::beg);
    position_ = start;
    return f;
  }
 public:
  bool done_;  // true if the iterator reached the end
  string curr_;  // the current ngram record;

  Impl() { Reset();}

  ~Impl() {
    for (int i = 0; i < files_.size(); ++i)
      delete files_[i].strm;
  }

  void Init(const string& args) {
    vector<string> options;
    SplitStringUsing(args, "\t", &options);
    string index_file = options[0];
    int p = index_file.find_last_of('/');
    if (p != string::npos) {
      dir_ = index_file.substr(0, p);
    } else {
      dir_ = ".";
    }

    string line;
    ifstream index_strm(index_file.c_str());
    int64 curr = 0;
    int len = 0;
    while (getline(index_strm, line)) {
      int tab = line.find_first_of('\t');
      assert(tab != string::npos);
      string key = line.substr(0, tab);
      int64 index = atoll(line.substr(tab + 1).c_str());
      if (index == 0) {
        vector<string> fields;
        SplitStringUsing(line, "\t", &fields);
        assert(fields.size() == 4);
        string filename = dir_ + "/" + fields[2];
        int64 size = atoll(fields[3].c_str());
        files_.push_back(FileInfo(filename, size));
      }
      len = files_.size();
      if (index == 0) {
        index_[key] = make_pair(len - 2, curr);
      } else {
        index_[key] = make_pair(len - 1, curr);
      }
      curr = index;
    }
    len = files_.size();
    last_ = make_pair(files_.size() - 1, curr);
    for (int i = 1; i < options.size(); ++i) {
      if (options[i] == "open-files-at-init") {
	for (int f = 0; f < files_.size(); ++f) {
	  string fname = files_[f].name;
	  files_[f].strm = new fstream(fname.c_str(), ios::in | ios::binary);
	}
      }
    }
  }

  // Clean up and get ready for the next search
  void Reset() {
    done_ = true;
    key_.clear();
    curr_.clear();
    locations_.clear();
    file_idx_ = -1;
    position_ = -1;
  }

  void Next() {
    if (done_)
      return;
    string line;
    while (file_idx_ >= 0) {
      while (position_ < files_[file_idx_].size) {
	getline(*files_[file_idx_].strm, line);
	position_ += line.size() + 1;  // +1 because getline strips
				       // out the ending \n
	assert(*files_[file_idx_].strm);
	int r = strncmp(key_.c_str(), line.c_str(), key_.size());
	if (r > 0) continue;
	if (r == 0 && isspace(line[key_.size()])) {
	  curr_ = line;
	} else
	  done_ = true;
	return;
      }
      file_idx_ = GetFile(0);  // Read from the beginning of the next
                               // file
    }
    done_ = true;
  }

  void Seek(const string& key) {
    Reset();
    key_ = key;
    done_ = false;
    map<string, pair<int, int64> >::const_iterator b = index_.upper_bound(key);
    int64 start = 0;
    int len = key.size();
    if (b == index_.end()) {
      locations_.push_back(last_.first);
      start = last_.second;
    } else {
      if (b->second.first < 0)
        return;
      locations_.push_back(b->second.first);
      start = b->second.second;
      int curr_file = b->second.first;
      while (++b != index_.end()) {
        if (b->second.first != curr_file) {
          locations_.push_back(b->second.first);
          curr_file = b->second.first;
        }
        if (strncmp(b->first.c_str(), key.c_str(), len) != 0)
          break;
      }
    }
    file_idx_ = GetFile(start);
    if (file_idx_ >= 0)
      Next();
  }
};

void NgramIterator::Init(const string& index_file) {
  impl_ = new NgramIterator::Impl;
  impl_->Init(index_file);
}

void NgramIterator::Next() {
  impl_->Next();
}

bool NgramIterator::Done() const {
  return impl_->done_;
}

void NgramIterator::Seek(const string& key) {
  impl_->Seek(key);
}

const string& NgramIterator::Current() const {
  return impl_->curr_;
}

NgramIterator::~NgramIterator() {
  delete impl_;
}
