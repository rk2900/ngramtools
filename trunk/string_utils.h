/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <vector>
#include <iostream>
#include <string>
#include <map>

#ifdef WS09
#include <stdarg.h>
#include <ext/hash_map>
#include <ext/hash_set>

using namespace __gnu_cxx;
using namespace std;

// Split the string by the separaters in set. The results are saved in fields.
void SplitStringUsing(const string& s, const char* sep, vector<string>* fields);

// Join the string in v with sep. The result is saved in s.
string JoinStringUsing(const vector<string>& v, const string& sep);

// Similar to printf, but creates a string instead
string StringPrintf(const char *fmt, ...);

typedef long long int64;

void ParseArgs(int argc, char* const argv[]);

void define_args_int(const char* arg, int* var, const char* comment);
void define_args_bool(const char* arg, bool* var, const char* comment);
void define_args_double(const char* arg, double* var, const char* comment);
void define_args_string(const char* arg, string* var, const char* comment);

#define DEFINE_int(arg, val, comment) int arg = (define_args_int(#arg, &arg, comment), val)
#define DEFINE_double(arg, val, comment) double arg = (define_args_double(#arg, &arg, comment), val)
#define DEFINE_string(arg, val, comment) string arg = (define_args_string(#arg, &arg, comment), val)
#define DEFINE_bool(arg, val, comment) string arg = (define_args_bool(#arg, &arg, comment), val)

#else

#include "strings/strutil.h"

#endif

string ToLower(const string& s);

unsigned long hashpjw(const char* s);

struct StringHash {
  unsigned long operator() (const string& key) const {
    return hashpjw(key.c_str());
  }
};

typedef hash_map<string, int64, StringHash> StrIntMap;

bool GetLineAndStrip(istream& istrm, string& line);

#endif
