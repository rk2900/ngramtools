#ifndef NGRAM_SERVER_H
#define NGRAM_SERVER_H

#include <string>
#include <cstdio>

using namespace std;

const string kEON = "<EON>";
const string kSearch = "s";
const string kSearchExact = "e";
const string kSearchAndFilter = "f";

const int kBufSize = 10000;
const int kLineBufSize = 1024;

class SocketBuffer {
  string buf_;
  char line_buf_[kLineBufSize + 1];
  int socket_;
  bool eof_;
public:
  bool eof() const { return eof_;}
  SocketBuffer(int socket = -1) { socket_ = socket; eof_ = false;}
  ~SocketBuffer() { if (socket_ >= 0) close(socket_);}
  void Socket(int s) { socket_ = s;}
  int Socket() { return socket_;}
  void Clear() { buf_.clear(); eof_ = false;}
  string GetNextLine() {
    string line;
    while (!eof_) {
      int p = buf_.find_first_of('\n');
      if (p != string::npos) {
	if (p >= 1 && buf_[p - 1] == '\r')
	  line = buf_.substr(0, p - 1);
	else 
	  line = buf_.substr(0, p);
	if (p + 1 == buf_.size())
	  buf_.clear();
	else
	  buf_ = buf_.substr(p + 1);
	break;
      }
      int len = ::read(socket_, line_buf_, kLineBufSize);
      if (len > 0) {
	line_buf_[len] = '\0';
	buf_ += string(line_buf_, len);
      } else {
	line = buf_;
	buf_.clear();
	eof_ = true;
	break;
      }
    }
    return line;
  }
};

#endif
