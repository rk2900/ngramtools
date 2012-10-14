/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

// This file define classes that allow pattern-matching on
// n-grams. For example, 

#ifndef NPATTERN_H
#define NPATTERN_H

#include <cmath>

#include "string_utils.h"
#include "ngram.h"
#include "vlist.h"

class NPattern;
class PrefixIterator;

// An NMatch object specifies a match between an NPattern and an
// N-gram
struct NMatch : public vector<NMatch*> {
  int fr;  // The starting position of the match (inclusive)
  int to;  // The ending position of the match (exclusive)
  int64 count;  // The counts of the fraction of ngram that matched
                // the pattern.  The reason this count may be smaller
                // than the ngram count is that some patterns
  NPattern* pat;  // The pattern responsible for this match. 
public:
  void NamedComponents(vector<const NMatch*>* named_matches) const;
  void ExtractKeyValPair(pair<const NMatch*, const NMatch*>* pair,
			 const string& key) const;
  void Prepend(NMatch* head);
  ostream& Print(ostream& ostrm);
  NMatch(int f, int t, int64 c) {
    fr = f; to = t; count = c;
    pat = 0;
  }
  bool EqualTo(const NMatch& m) const;
  bool ShallowEqualTo(const NMatch& m) const;
  NMatch* Project();
  NMatch* ShallowCopy() const;
  NMatch* DeepCopy() const;
  ~NMatch();
};

class NMatchVec : public vector<NMatch*> {
public:
  void Add(NMatch* m, NPattern* pat) {
    push_back(m);
    m->pat = pat;
  }
  ostream& Print(ostream& ostrm) {
    for (int i = 0; i < size(); ++i) {
      (*this)[i]->Print(ostrm);
      ostrm << endl;
    }
    return ostrm;
  }
  ~NMatchVec();
};

class NPattern {
public:
  virtual ~NPattern() {}
  virtual void Init(const Vlist& args) {};

  // Return true if the ngram (as a whole) matches this pattern
  virtual bool Match(const Ngram& ngram, NMatchVec* matches = 0);

  string Name() const { return name_;}

  // Create an instance of NPattern of the same type as this
  virtual NPattern* clone() const = 0;

  // Finding matches of this pattern starting at the given position of
  // the ngram.
  virtual void PartialMatch(const Ngram& ngram, int position,
			    NMatchVec* matches) = 0;

  // Register a new type of NPattern
  static void InitProtos();
  static void RegisterProto(const string& type, NPattern* proto);
  static NPattern* Create(const Vlist& args);
  static NPattern* Create(const string& args);
  int min_len() const { return min_len_;}
  int max_len() const { return max_len_;}
  NPattern() { min_len_ = 1; max_len_ = 1;}
protected:
  string name_;
  int min_len_; // inclusive
  int max_len_; // inclusive

  static hash_map<string, NPattern*, StringHash> protos;
};

class NgramInfoCollector {
public:
  class Printer {
   public:
    virtual ~Printer() {}
    virtual void Print(const string& key) {
      cout << key;
    }
  };

  NgramInfoCollector() {
    pat_ = 0;
    max_match_ = 0; log_count_ = false; uniq_count_ = false;
    printer_ = 0;
    allow_partial_match_ = false;
  }
  virtual ~NgramInfoCollector() { delete pat_; delete printer_;}

  virtual void Init(const Vlist& args);
  virtual void Start() {};
  virtual void Flush() {};

  // Collect the information in the ngram.
  virtual bool CollectInfo(const Ngram& ngram) = 0;

  // Run CollectInfo on all ngrams that matches the given prefix
  virtual void CollectAll(PrefixIterator* it);

  virtual NgramInfoCollector* clone() const = 0;

  void set_printer(Printer* p) { printer_ = p;}
  Printer* printer() {
    if (printer_ == 0)
      printer_ = new Printer;
    return printer_;
  }
  void set_prefix_query(const string& prefix_query) {
    prefix_query_ = prefix_query;
  }
  const string& prefix_query() const { return prefix_query_;}
  int max_match() const { return max_match_;}

  int min_len() const { return min_len_;}
  int max_len() const { return max_len_;}

  int64 GetCount(int64 count) const {
    if (log_count_)
      return (int) log(count) + 1;
    else if (uniq_count_)
      return 1;
    else
      return count;
  }

  static void RegisterProto(const string& type, NgramInfoCollector* proto);
  static NgramInfoCollector* Create(const Vlist& args);
  static NgramInfoCollector* Create(const string& args);
protected:
  Printer* printer_;
  NPattern* pat_;
  string prefix_query_;
  int max_match_;
  bool log_count_;
  bool uniq_count_;
  int min_len_; // inclusive
  int max_len_; // inclusive
  bool allow_partial_match_;
  static hash_map<string, NgramInfoCollector*, StringHash> protos;
};

#endif
