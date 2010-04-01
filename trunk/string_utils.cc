/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include <iostream>
#include <cassert>

#include "string_utils.h"

#ifdef WS09
void SplitStringUsing(const string& s, const char* sep,
		      vector<string>* fields) {
  int last = 0;
  for (int i = 0; i < s.size(); ++i) {
    if (strchr(sep, s[i])) {
      if (i > last)
        fields->push_back(s.substr(last, i - last));
      last = i + 1;
    }
  }
  if (last < s.size())
    fields->push_back(s.substr(last));
}

string JoinStringUsing(const vector<string>& v, const string& sep) {
  string out;
  for (int i = 0; i < v.size(); ++i) {
    if (!out.empty())
      out += sep;
    out += v[i];
  }
  return out;
}

string StringPrintf(const char *fmt, ...) {
  /* Guess we need no more than 100 bytes. */
  int n, size = 100;
  char *p, *np;
  va_list ap;
  p = (char*) malloc(size);
  assert(p);
  while (1) {
    /* Try to print in the allocated space. */
    va_start(ap, fmt);
    n = vsnprintf (p, size, fmt, ap);
    va_end(ap);
    /* If that worked, return the string. */
    if (n > -1 && n < size) {
      break;
    }
    /* Else try again with more space. */
    if (n > -1)    /* glibc 2.1 */
      size = n + 1; /* precisely what is needed */
    else           /* glibc 2.0 */
      size *= 2;  /* twice the old size */
    np = (char*) realloc (p, size);
    assert(np);
    p = np;
  }
  string s = p;
  free(p);
  return s;
}


struct Rec {
  string comment;
  int* int_var;
  double* double_var;
  bool* bool_var;
  string* string_var;

  Rec() { int_var = 0; bool_var = 0; double_var = 0; string_var = 0;}
};

static map<string, Rec> args;

void ParseArgs(int argc, char* const argv[]) {
  for (int i = 1; i < argc; ++i) {
    string arg_name = argv[i], arg_val = "true";
    if (argv[i][0] == '-') {
      string s = argv[i];
      int p = s.find_first_not_of('-');
      int q = s.find_first_of('=');
      if (q == string::npos) {
	arg_name = s.substr(p);
	if (i + 1 < argc && argv[i + 1][0] != '-') {
	  arg_val = argv[++i];
	}
      } else {
	arg_name = s.substr(p, q - p);
	arg_val = s.substr(q + 1);
      }
    }
    if (args.find(arg_name) == args.end()) {
      cerr << arg_name << " is not a declared command line argument" << endl;
      exit(1);
    }
    Rec& r = args[arg_name];
    if (r.int_var)
      *r.int_var = atoi(arg_val.c_str());
    else if (r.double_var)
      *r.double_var = atof(arg_val.c_str());
    else if (r.bool_var)
      *r.bool_var = (arg_val == "true")? true : false;
    else if (r.string_var)
      *r.string_var = arg_val;
  }
}

void CheckUndefined(string name) {
  if (args.find(name) != args.end()) {
    cerr << "The flag " << name << " has multiple definitions." << endl;
    exit(1);
  }
}

void define_args_int(const char* arg, int* var, const char* comment) {
  CheckUndefined(arg);
  Rec& r = args[arg];
  r.comment = comment;
  r.int_var = var;
}  

void define_args_bool(const char* arg, bool* var, const char* comment) {
  CheckUndefined(arg);
  Rec& r = args[arg];
  r.comment = comment;
  r.bool_var = var;
}

void define_args_double(const char* arg, double* var, const char* comment) {
  CheckUndefined(arg);
  Rec& r = args[arg];
  r.comment = comment;
  r.double_var = var;
}

void define_args_string(const char* arg, string* var, const char* comment) {
  CheckUndefined(arg);
  Rec& r = args[arg];
  r.comment = comment;
  r.string_var = var;
}
#endif

string ToLower(const string& s){
  string local =s;

  int i=0;
  for(i=0;i<s.size();i++){
    local[i]=tolower(s[i]);
 }
  return local;
}

unsigned long hashpjw(const char* x) // From Dragon book, p436
{
  unsigned long h = 0;
  unsigned long g;
  unsigned char ch;
  while (*x != 0) {
    ch = (unsigned char) *x++;
    if ((ch&0200)==0)
      ch = tolower(ch);
    h = (h << 4) + ch;
    if ((g = h & 0xf0000000L) != 0)
      h = (h ^ (g >> 24)) ^ g;
  }
  return h;
}

bool GetLineAndStrip(istream& istrm, string& line) {
  bool r = getline(istrm, line);
  int p = line.find_last_not_of(" \t\r\n");
  line.resize(p + 1);
  return r;
}
