/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#ifndef NGRAM_H
#define NGRAM_H

#include "string_utils.h"

const int kMaxNgramLen = 10;

// For example, ngram.TagSeq(j).seq_[i] is the tag at ith position in
// jth tag sequence
class Ngram {
public:
  Ngram() { divider_ = 0; buf_ = 0; buf_size_ = 0;}
  ~Ngram() { delete [] buf_;}

  // Initialize an ngram with a line in the ngram file (either rotated
  // or not).
  void Init(const string& line);

  string Concat(int fr, int to) const;
  int64 TokenCount() const { return counts_[0];}
  int Length() const { return n_;}

  const char* Token(int position) const {
    return strs_[position];
  }

  int TagSeqLen() const { return tagseqs_;}

  const char* Tag(int i, int j) const { return strs_[(i + 1) * n_ + j]; }
  int64 TagCount(int i) const { return counts_[i + 1];}
  void Print(ostream& ostrm) const;
  int Divider() const { return divider_;}
  void Clear();
  const string& orig() const { return orig_;}

  static int Length(const string& s);
protected:
  string orig_; // the original n-gram record
  vector<const char*> strs_;
  char* buf_; // the buffer containing the contents of strs_;
  int buf_size_; //
  int n_;  // the N in an n-gram.
  int tagseqs_; // the number of tag sequences for this n-gram
  vector<int64> counts_; // counts_[0] is the n-gram count, counts_[1]
			 // to counts_[tagseqs_] are the counts of tag
			 // sequences
  int divider_;  // the position of the divider ><, the tokens at
		 // position [0, divider_) were after >< in the
		 // rotated ngram file.
};

#endif
