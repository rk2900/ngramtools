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

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cerr << "Usage: search_prefix INDEX_FILE|HOST_AND_PORT [QUERY [COMMAND]]" << endl;
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
    while (getline(cin, line)) {
      for (it->Seek(line); !it->Done(); it->Next()) {
	cout << it->Current() << endl;
      }
    }
  } else {
    NgramInfoCollector* col = 0;
    if (argc > 3) {
      col = NgramInfoCollector::Create(argv[3]);
      col->set_prefix_query(argv[2]);
    } else if (argc == 3 && argv[2][0] == '(')
      col = NgramInfoCollector::Create(argv[3]);
    if (col)
      col->CollectAll(it);
    else {
      for (it->Seek(argv[2]); !it->Done(); it->Next()) {
	cout << it->Current() << endl;
      }
    }
    delete col;
  }
  delete it;
}
