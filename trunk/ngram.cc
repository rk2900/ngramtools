/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include "ngram.h"

const string kDivider = "><";
const int kExtra = 20;

void Ngram::Print(ostream& ostrm) const {
  ostrm << divider_ << '\t';
  for (int i = 0; i <= strs_.size(); ++i) {
    if (i > 0 || i < n_)
      ostrm << ' ';
    else if ((i % n_) == 0) {
      if (i > 0)
	ostrm << '\t' << counts_[(i - 1) / n_];
      if (i == strs_.size())
	break;
    } else {
      ostrm << '|';
    }
    if (i < strs_.size())
      ostrm << strs_[i];
  }
  ostrm << endl;
}  

string Ngram::Concat(int fr, int to) const {
  string str;
  for (int i = fr; i < to; ++i) {
    if (i > fr)
      str += " ";
    str += strs_[i];
  }
  return str;
}

void Ngram::Init(const string& line) {
  Clear();
  orig_ = line;
  if (line.size() >= buf_size_) {
    delete [] buf_;
    buf_size_ = line.size() + kExtra;
    buf_ = new char[buf_size_];
  }
  memcpy(buf_, line.c_str(), line.size() + 1);
  n_ = 0;
  tagseqs_ = 0;
  strs_.reserve(12);
  counts_.reserve(4);
  char* p = buf_;
  const char* prev = buf_;
  int after_divider = -1;
  for (; *p && *p != '\t'; ++p) {
    if (*p == ' ') {
      *p = '\0';
      if (kDivider != prev) {
	++n_;
	strs_.push_back(prev);
      } else
	after_divider = n_;
      prev = p + 1;
    }
  }
  if (*p == '\t') {
    *p++ = '\0';
    strs_.push_back(prev);
    prev = p;
    ++n_;
  }
  if (after_divider == -1)
    divider_ = 0;
  else {  // This is a rotated n-gram. We need to normalize the order
    divider_ = n_ - after_divider;
    vector<const char*> tmp(divider_);
    for (int k = 1; k <= divider_; ++k)
      tmp[k - 1] = strs_[n_ - k];
    for (int k = after_divider - 1; k >= 0; --k)
      strs_[k + divider_] = strs_[k];
    for (int k = 0; k < divider_; ++k)
      strs_[k] = tmp[k];
  }
  for (; *p && *p != '\t' && *p != '\n'; ++p)
    ;
  counts_.push_back(atoll(prev));
  if (*p++ != '\t')
    return;
  *p++ = '\0';
  // skip past ' '
  for (; *p && *p != ' '; ++p)
    ;
  prev = ++p;
  while (*p && *p != '\t' && *p != '\n') {
    for (; *p && *p != ' ' && *p != '\t'; ++p) {
      if (*p == '|') {
	*p = '\0';
	strs_.push_back(prev);
	prev = p + 1;
      }
    }
    char last_char = *p;
    if (*p)
      *p++ = '\0';
    counts_.push_back(atoll(prev));
    prev = p;
    ++tagseqs_;
    if (last_char != ' ')
      break;
  }
}

void Ngram::Clear() {
  counts_.clear();
  strs_.clear();
  orig_.clear();
  n_ = 0;
  tagseqs_ = 0;
  counts_.clear();
  divider_ = 0;
}

int Ngram::Length(const string& s) {
  if (s.empty())
    return 0;
  int len = 1, slen = s.size();
  for (int i = 0; i < slen; ++i) {
    if (s[i] == '\t')
      break;
    else if (s[i] == ' ')
      ++len;
    else if (s[i] == '>' && i > 0 && i + 2 < slen && s[i - 1] == ' ' && s[i + 1] == '<' && s[i + 2] == ' ')
      --len;
  }
  return len;
}
