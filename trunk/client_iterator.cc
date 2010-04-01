/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include <iostream>
#include <assert.h>
#include <arpa/inet.h>

#include "ngram_server.h"
#include "client_iterator.h"
#include "string_utils.h"
#include "sock.h"

typedef long long int64;  // using int64 for index just in case some
                          // files are bigger than 4GB

class ClientIterator::Impl {
  string server_and_port_;
  SocketBuffer sock_;
  string key_;  // the current key being searched
 public:
  bool done_;  // true if the iterator reached the end
  string curr_;  // the current ngram record;

  Impl() {
    Reset();
    sock_ = -1;
  }

  void Init(const string& server_and_port) {
    server_and_port_ = server_and_port;
    int p = server_and_port_.find_last_of(':');
    string server, port;
    if (p != string::npos) {
      server = server_and_port_.substr(0, p);
      port = server_and_port_.substr(p + 1);
    }
    int socket = make_connection(port.c_str(), SOCK_STREAM, server.c_str());
    sock_.Socket(socket);
  }

  // Clean up and get ready for the next search
  void Reset() {
    done_ = true;
    curr_.clear();
    key_.clear();
    sock_.Clear();
  }

  void Next() {
    curr_ = sock_.GetNextLine();
    if (curr_ == kEON || curr_ == "")
      done_ = true;
  }

  void Seek(const string& key) {
    key_ = key;
    done_ = false;
    for (int i = 0; i < key_.size(); ++i) {
      if (key_[i] == '\n' || key_[i] == '\r')
	key_[i] = ' ';
    }
    string k = key_ + "\n";
    write(sock_.Socket(), k.c_str(), k.size());
    Next();
    if (done_)
      Reset();
  }
};

void ClientIterator::Init(const string& args) {
  impl_ = new ClientIterator::Impl;
  impl_->Init(args);
}

void ClientIterator::Next() {
  impl_->Next();
  if (impl_->done_)
    impl_->Reset();
}

bool ClientIterator::Done() const {
  return impl_->done_;
}

string ClientIterator::Search(const string& key) {
  impl_->Seek(string("e\t") + key);
  string r = impl_->curr_;
  while (!Done())
    Next();
  return r;
}

void ClientIterator::Seek(const string& key) {
  impl_->Seek(key);
}

const string& ClientIterator::Current() const {
  return impl_->curr_;
}

ClientIterator::~ClientIterator() {
  delete impl_;
}
