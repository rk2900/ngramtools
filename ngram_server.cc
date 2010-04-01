/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include <iostream>
#include <string>
#include <cstdio>
#include <sys/socket.h>
#include <sys/wait.h>
#include <cstring>

#include "ngram_iterator.h"

#ifdef sun
#ifdef  __cplusplus
extern "C" {
#endif

#if defined(__STDC__)
pid_t wait3(int *statusp, int options, struct rusage *rusage);
#endif  /* __STDC__ */

#ifdef  __cplusplus
}
#endif
#endif /* sun */

#include "sock.h"
#include "vlist.h"
#include "npattern.h"
#include "ngram_server.h"

using namespace std;

char DEFAULT_PORT[32]="6700";

int listensock = -1; /* So that we can close sockets on ctrl-c */

/* This waits for all children, so that they don't become zombies. */

void sig_chld(int)
{
  int pid;
  int status;
  while ( (pid = wait3(&status, WNOHANG, NULL)) > 0);
}

int main(int argc, char *argv[]) {
  const char* port = DEFAULT_PORT;
  if (argc > 2) {
    port = argv[2];
  }

  int ports = -1;
  struct sigaction act, oldact;

  ignore_pipe();
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = sig_chld;
  sigaction(SIGCHLD, &act, &oldact);
  ports = atoport(port, "tcp");
  if (ports == -1) {
    fprintf(stderr,"Unable to find service\n");
    exit(EXIT_FAILURE);
  }
  NgramIterator it;
  it.Init(string(argv[1]) + "\topen-files-at-init");
  cerr << "Ready to serve!" << endl;
  int sock = get_connection(SOCK_STREAM, ports, &listensock);
  string input, out;
  SocketBuffer buf(sock);
  while (!buf.eof()) {
    string line = buf.GetNextLine();
    int p = line.find_first_of("\t");
    string cmd, arg;
    if (p == string::npos) {
      cmd = kSearch;
      arg = line;
    } else {
      cmd = line.substr(0, p);
      arg = line.substr(p + 1);
    }
    if (cmd == kSearch) {
      for (it.Seek(arg); !it.Done(); it.Next()) {
	out += it.Current() + "\n";
	if (out.size() > kBufSize) {
	  write(sock, out.c_str(), out.size());
	  out.clear();
	}
      }
    } else if (cmd == kSearchExact) {
      vector<string> keys;
      SplitStringUsing(arg, "\t", &keys);
      for (int i = 0; i < keys.size(); ++i) {
	it.Seek(keys[i]);
	if (!it.Done()) {
	  out += it.Current() + "\n";
	}
      }
    } else if (cmd == kSearchAndFilter) {
      Vlist cmd;
      cmd.Read(arg);
      NgramInfoCollector* col = NgramInfoCollector::Create(cmd);
      if (col) {
	col->CollectAll(&it);
	delete col;
      }
    }
    out += kEON + "\n";
    write(sock, out.c_str(), out.size());
    out.clear();
  }
}
