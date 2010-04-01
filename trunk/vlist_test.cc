/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include <iostream>
#include <cassert>

#include "vlist.h"

using namespace std;

int main() {
  Vlist v;
//  v.Read("(This is (() (small \" test \\n test\")))");
//  assert(v[0]->size() == 3);
//  assert(v[0]->str(0) == "This");
//  assert((*v[0])[2]->size() == 2);
//  assert(v[0]->Get("is")->size() == 2);
  v.Read(cin);
  v.Print(cerr);
  cerr << endl;
}
