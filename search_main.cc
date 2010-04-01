/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include <iostream>

#include "ngram_iterator.h"
#include "client_iterator.h"
#include "npattern.h"
#include "regulare.h"

inline bool SameKey(const string& ngram, const string& key) {
  int len = key.size();
  return ngram.size() > len && ngram[len] == '\t';
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cerr << "Usage: search INDEX_FILE|HOST_AND_PORT [QUERY [COMMAND]]" << endl;
    exit(1);
  }
  PrefixIterator* it = 0;
  RegExp pat(".*:[0-9]+");
  if (pat.exact_match(argv[1]))
    it = new ClientIterator;
  else
    it = new NgramIterator;
  it->Init(argv[1]);
  if (argc == 2) {
    string line;
    while (GetLineAndStrip(cin, line)) {
      for (it->Seek(line); !it->Done(); it->Next()) {
        if (SameKey(it->Current(), line))
          cout << it->Current() << endl;
        else
          break;
      }
    }
  } else {
    NgramInfoCollector* col = 0;
    for (it->Seek(argv[2]); !it->Done(); it->Next()) {
      if (SameKey(it->Current(), argv[2]))
	cout << it->Current() << endl;
      else
        break;
    }
  }
  delete it;
}
