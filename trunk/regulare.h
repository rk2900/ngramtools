#ifndef REGULAREXP_H
#define REGULAREXP_H

//      CLASS RegExp
//  A regular expression class
//
//  Interface:
//  RegExp():
//  RegExp(char* pattern):
//      Constructors for regular expression object, with
//      and without a pattern.
//  void reset(char* pattern):
//      Compile the pattern into _impl.
//  int match(const char* str);
//      Match the beginning of str against the regular
//      expression in _impl. If no match, return -1.
//      Otherwise the return value is the length of the
//      substring of str that matches the regular expression.
//  int exact_match(const char* str);
//      Return 1 if the whole 'str' matches the regular
//      expression. Otherwise, return 0.

struct regexp;

class RegExp {
  regexp* _impl;
  char* _replacement;
public:
  RegExp() { _impl = 0; _replacement = 0;}
  RegExp(const RegExp& e);
  RegExp(const char* pattern, int prefix = 1, const char* replacement = 0);
  ~RegExp();

  /**
    perform substitutions after a regexp match
    */
  void substitute(const char* source, char* destination, int global);

  void reset(const char* pattern, int prefix = 1);
  
  /**
    return i if str[0,i] matches the regular expression
    return -1 otherwise
   */
  int match(const char* str) const;

  /**
    return 1 if the complete string matches the regular expression
    return 0 otherwise
   */
  int exact_match(const char* str) const;
};

#endif
