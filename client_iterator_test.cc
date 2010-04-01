/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include <iostream>

#include "client_iterator.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    cerr << "Usage: search_prefix INDEX_FILE" << endl;
    exit(1);
  }
  PrefixIterator* it = new ClientIterator;
  it->Init(argv[1]);
  string line;
  cout << "> " << flush;
  while (getline(cin, line)) {
    for (it->Seek(line); !it->Done(); it->Next()) {
      cout << it->Current() << endl;
    }
    cout << "> " << flush;
  }
}
