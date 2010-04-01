/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#ifndef VLIST_H
#define VLIST_H

#include "string_utils.h"

class Vlist : public vector<Vlist*> {
public:
  void ReadList(istream& istrm);
  bool Read(istream& istrm);
  bool Read(const string& str);

  bool Atom() const;
  void Clear();
  ostream& Print(ostream& ostrm) const;
  const Vlist* Get(const string& label) const;
  bool HasLabel(const string& label) const;
  string GetAtom(const string& label) const;
  const string& str() const { return str_;}
  const string& str(int i) const { return (*this)[i]->str_;}
  ~Vlist();
protected:
  string str_;
};

#endif
