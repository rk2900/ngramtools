#include <iostream>
#include <string>
using namespace std;

class ReducerInput {
public:
  bool Done() const { return done_;}
  bool AllDone() const { return all_done_;}
  ReducerInput() {
    all_done_ = false;
    done_ = false;
    Next();
    if (!all_done_)
      UnsetDone();
  }
  void SkipRest() {
    while (!Done())
      Next();
  }
  void Next() {
    string line;
    if (!getline(cin, line)) {
      done_ = true;
      all_done_ = true;
    } else {
      int p = line.find_first_of('\t');
      if (p == string::npos) {
	new_key_ = line;
	value_.clear();
      } else {
	new_key_ = line.substr(0, p);
	value_ = line.substr(p + 1);
      }
      if (new_key_ != key_) {
	done_ = true;
      }
    }
  }
  void UnsetDone() {
    done_ = false;
    key_ = new_key_;
  }
  const string& key() const { return key_;}
  const string& value() const { return value_;}
protected:
  string key_;
  string value_;
  string new_key_;
  bool done_;
  bool all_done_;
};
