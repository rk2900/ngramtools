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
#include "string_utils.h"
#include "sock.h"

void Init(const string& server_and_port, SocketBuffer* socket) {
  int p = server_and_port.find_last_of(':');
  string server, port;
  if (p != string::npos) {
    server = server_and_port.substr(0, p);
    port = server_and_port.substr(p + 1);
  }
  int s = make_connection(port.c_str(), SOCK_STREAM, server.c_str());
  socket->Socket(s);
}

void Seek(string key, SocketBuffer* socket) {
  for (int i = 0; i < key.size(); ++i) {
    if (key[i] == '\n' || key[i] == '\r')
      key[i] = ' ';
  }
  string k = key + "\n";
  write(socket->Socket(), k.c_str(), k.size());
};

int main(int argc, char* argv[]) {
  vector<string> results;
  for (int i = 0; i < 10; ++i) {
    SocketBuffer b;
    Init(argv[1], &b);
    for (int j = 0; j < 100; ++j) {
      Seek(argv[2], &b);
      int k = 0;
      while (1) {
	string line = b.GetNextLine();
	if (line == kEON)
	  break;
	if (i == 0 && j == 0) {
	  results.push_back(line);
	  cerr << line << endl;
	} else if (line != results[k++]) {
	  cerr << "Error!" << endl;
	  cerr << "line=" << line << endl;
	  cerr << " vs. " << results[k - 1] << endl;
	}
      }
    }
  }
}
