#include <cstring>
#include <iostream>
#include "string_utils.h"
#include "regulare.h"
#include "regexp.h"

inline char* dupstring(const char* str)
{
  if (str==0)
    return 0;
  char* dup = new char[strlen(str)+1];
  strcpy(dup, str);
  return dup;
}

RegExp::RegExp(const char* pattern, int prefix, const char* replacement)
{
  _impl = 0;
  reset(pattern, prefix);
  if (replacement)
    _replacement = dupstring(replacement);
  else
    _replacement = 0;
}

RegExp::RegExp(const RegExp&)
{
  _impl = 0;
  _replacement = 0;
  cerr << "Sorry: duplication of regular expression is not supported yet\n";
}

int RegExp::exact_match(const char* text) const
{
  int len = match(text);
  return (len>0) && (len == (int) strlen(text));
}

int RegExp::match(const char* str) const
{
  if (str==0 || _impl==0 || regexec(_impl, (char*) str)==0)
    return -1;
  else
    return _impl->endp[0]-str;
}

void RegExp::substitute(const char* source, char* destination, int global)
{
  if (source==0 || destination==0 || _impl==0 || _replacement==0)
    return;
  char* d = destination;
  char* s = (char*) source;
  while (regexec(_impl, s)) {
    for (;s!=_impl->startp[0]; s++, d++)
      *d = *s;
    regsub(_impl, (char*) _replacement, d);
    d += strlen(d);
    s = _impl->endp[0];
    if (!global)
      break;
  }
  strcpy(d, s);
}

void RegExp::reset(const char* pattern, int prefix)
{
  if (_impl) 
    delete [] _impl;
  if (pattern==0)
    _impl=0;
  else {
    if (*pattern=='^' || prefix==0)
      _impl = regcomp((char*) pattern);
    else {
      string buf = string("^") + pattern;
      _impl = regcomp((char*) buf.c_str());
    }
  }    
}

RegExp::~RegExp()
{
  delete [] _impl;
}
