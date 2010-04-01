/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include <sstream>
#include <iostream>

#include "vlist.h"

void Vlist::ReadList(istream& istrm) {
  Vlist* sub = new Vlist;
  while (sub->Read(istrm)) {
    push_back(sub);
    sub = new Vlist;
  }
  delete sub;
}

bool Vlist::Read(istream& istrm) {
  char ch;
  while (istrm.get(ch)) {
    if (ch == ';') {
      while (istrm.get(ch) && ch != '\n')
	;
    } else if (!isspace(ch))
      break;
  }
  if (!istrm|| ch == ')') {
    return false;
  }
  if (ch == '(') {
    ReadList(istrm);
  } else {
    string atom;
    if (ch == '"') { // quoted atom
      while (istrm.get(ch)) {
	if (ch == '"')
	  break;
	if (ch == '\\') {
	  istrm.get(ch);
	  if (ch == 'n')
	    ch = '\n';
	  else if (ch == 't')
	    ch = '\t';
	}
	atom += ch;
      }
    } else { // normal atom
      do {
	if (ch == '(' || ch == ')' || ch == '"' || ch == ';') {
	  istrm.putback(ch);
	  break;
	} else if (isspace(ch))
	  break;
	atom += ch;
      } while (istrm.get(ch));
    }
    str_ = atom;
  }
  return true;
}

bool Vlist::Atom() const {
  return size() == 0 && str() != "";
}

Vlist::~Vlist() {
  for (int i = 0; i < size(); ++i)
    delete (*this)[i];
}

void Vlist::Clear() {
  for (int i = 0; i < size(); ++i) {
    delete (*this)[i];
  }
  clear();
}

bool Vlist::Read(const string& str) {
  istringstream istrm(str);
  return Read(istrm);
}

ostream& Vlist::Print(ostream& ostrm) const {
  if (size() > 0) {
    ostrm << "(";
    for (int i = 0; i < size(); ++i) {
      (*this)[i]->Print(ostrm);
      ostrm << ' ';
    }
    ostrm << ")";
  } else if (!str_.empty()) {
    ostrm << str_;
  } else {
    ostrm << "()";
  }
  return ostrm;
}

const Vlist* Vlist::Get(const string& label) const {
  bool found = false;
  for (int i = 0; i < size(); ++i) {
    const Vlist* element = (*this)[i];
    if (found)
      return element;
    if (element->str() == label)
      found = true;
  }
  return 0;
}

string Vlist::GetAtom(const string& label) const {
  bool found = false;
  for (int i = 0; i < size(); ++i) {
    const Vlist* element = (*this)[i];
    if (found)
      return element->str();
    if (element->str() == label)
      found = true;
  }
  return "";
}

bool Vlist::HasLabel(const string& label) const {
  for (int i = 0; i < size(); ++i) {
    if ((*this)[i]->str() == label)
      return true;
  }
  return false;
}
