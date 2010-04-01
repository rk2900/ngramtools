/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#include <cassert>
#include <fstream>
#include "npattern.h"
#include "regulare.h"
#include "prefix_iterator.h"

class Trie : public map<string, Trie*> {
public:
  Trie() { is_end_ = false;}

  bool is_end() const { return is_end_;}

  void LookUpFrom(const vector<string>& tokens, int pos, vector<int>* results) {
    if (is_end_)
      results->push_back(pos);
    if (pos == tokens.size())
      return;
    const string& token = tokens[pos];
    map<string, Trie*>::iterator it = find(token);
    if (it != end())
      return it->second->LookUpFrom(tokens, pos + 1, results);
  }

  bool AddEntry(const string& phrase) {
    vector<string> tokens;
    SplitStringUsing(phrase, " ", &tokens);
    return AddEntry(tokens);
  }
  bool AddEntry(const vector<string>& tokens) {
    if (tokens.empty())
      return false;
    else
      return AddEntryAux(tokens, 0);
  }
protected:
  bool is_end_;

  bool AddEntryAux(const vector<string>& tokens, int pos) {
    if (pos == tokens.size()) {
      if (is_end_)
	return false;
      else
	return is_end_ = true;
    }
    const string& token = tokens[pos];
    map<string, Trie*>::iterator it = find(token);
    if (it == end()) {
      Trie* next = new Trie;
      (*this)[token] = next;
      return next->AddEntryAux(tokens, pos + 1);
    } else {
      return it->second->AddEntryAux(tokens, pos + 1);
    }
  }
};

bool NMatch::EqualTo(const NMatch& m) const {
  if (m.size() != size() || m.fr != fr || m.to != to)
    return false;
  for (int i = 0; i < size(); ++i) {
    if (!(*this)[i]->EqualTo(*m[i]))
      return false;
  }
  return true;
}

bool NMatch::ShallowEqualTo(const NMatch& m) const {
  return m.fr == fr && m.to == to;
}

NMatch* NMatch::Project() {
  NMatch* m = new NMatch(fr, to, count);
  m->pat = pat;
  m->push_back(this);
  return m;
}

NMatch* NMatch::ShallowCopy() const {
  NMatch* m = new NMatch(fr, to, count);
  m->pat = pat;
  return m;
}

NMatch* NMatch::DeepCopy() const {
  NMatch* m = ShallowCopy();
  int s =  size();
  for (int i = 0; i < s; ++i) {
    m->push_back((*this)[i]->DeepCopy());
  }
  return m;
}

ostream& NMatch::Print(ostream& ostrm) {
  ostrm << "(" << fr << "," << to << " " << count;
  if (pat && !pat->Name().empty())
    ostrm << " " << pat->Name();
  for (int i = 0; i < size(); ++i) {
    ostrm << ' ';
    (*this)[i]->Print(ostrm);
  }
  ostrm << ")";
  return ostrm;
}

void NMatch::Prepend(NMatch* head) {
  assert(head->to == fr);
//  cerr << "head:";
//  head->Print(cerr);
//  cerr << endl;
//  cerr << "before:";
//  Print(cerr);
//  cerr << endl;
  fr = head->fr;
  if (head->count > 0 && count > head->count)
    count = head->count;
  else if (count < 0)
    count = head->count;
  resize(size() + 1);
  for (int i = size() - 2; i >= 0; --i)
    (*this)[i + 1] = (*this)[i];
  (*this)[0] = head;
//  cerr << "after:";
//  Print(cerr);
//  cerr << endl;
}

NMatchVec::~NMatchVec() {
  for (int i = 0; i < size(); ++i)
    delete (*this)[i];
}

NPattern* NPattern::Create(const string& args) {
  Vlist v;
  v.Read(args);
  return NPattern::Create(v);
}

NPattern* NPattern::Create(const Vlist& args) {
  const NPattern* proto = protos[args[0]->str()];
  if (proto == 0)
    cerr << "Unknown pattern type: " << args[0]->str() << endl;
  assert(proto != 0);
  NPattern* p = proto->clone();
  p->Init(args);
  const Vlist* arg = args.Get(":name");
  if (arg)
    p->name_ = arg->str();
  return p;
}

NMatch::~NMatch() {
  for (int i = 0; i < size(); ++i) {
    delete (*this)[i];
  }
}

// This pattern matches any position in an n-gram
class TPattern : public NPattern {
public:
  NPattern* clone() const { return new TPattern;}
  virtual void PartialMatch(const Ngram& ngram, int position,
			    NMatchVec* matches) {
    if (position < ngram.Length())
      matches->Add(new NMatch(position, position + 1, ngram.TokenCount()),
		   this);
  }
};

// This pattern matches the token at the divider position
class DividerPattern : public NPattern {
public:
  NPattern* clone() const { return new DividerPattern;}
  virtual void PartialMatch(const Ngram& ngram, int position,
			    NMatchVec* matches) {
    if (position == ngram.Divider())
      matches->Add(new NMatch(position, position + 1, ngram.TokenCount()),
		   this);
  }
};

// This pattern matches any position in an n-gram
class GrepPattern : public NPattern {
public:
  NPattern* clone() const { return new GrepPattern;}
  GrepPattern() { regexp_ = 0; min_len_ = 1; max_len_ = kMaxNgramLen;}
  ~GrepPattern() { delete regexp_;}
  virtual void Init(const Vlist& args) {
    regexp_ = new RegExp(args.str(1).c_str());
  }
  virtual void PartialMatch(const Ngram& ngram, int position,
			    NMatchVec* matches) {
    if (position == 0 && regexp_->exact_match(ngram.orig().c_str())) {
      matches->Add(new NMatch(0, ngram.Length(), ngram.TokenCount()), this);
    }
  }
 protected:
  RegExp* regexp_;
};

class WordPattern : public NPattern {
public:
  WordPattern() { regexp_ = 0; wordlist_ = 0;}
  NPattern* clone() const { return new WordPattern;}
  ~WordPattern() {
    delete regexp_;
    delete wordlist_;
  }
  virtual void Init(const Vlist& args) {
    assert(args.size() > 2);
    if (args.str(1) == "=") {
      word_ = args.str(2);
    } else if (args.str(1) == "~") {
      regexp_ = new RegExp(args.str(2).c_str());
    } else if (args.str(1) == "in") {
      wordlist_ = new hash_set<string, StringHash>;
      for (int i = 2; i < args.size(); ++i) {
	if (args[i]->str().empty()) {
	  for (int j = 0; j < args[i]->size(); ++j) {
	    const string& s = args[i]->str(j);
	    if (!s.empty())
	      wordlist_->insert(s);
	  }
	} else {
	  ifstream file(args[i]->str().c_str());
	  string line;
	  while (getline(file, line))
	    wordlist_->insert(line);	  
	}
      }
    }
  }
  virtual void PartialMatch(const Ngram& ngram, int position,
			    NMatchVec* matches) {
    if (position >= ngram.Length())
      return;
    const char* token = ngram.Token(position);
    bool found = false;
    if (regexp_ != 0) {
      found = regexp_->exact_match(token);
    } else if (wordlist_) {
      found = (wordlist_->find(token) != wordlist_->end());
    } else {
      found = (word_ == token);
    }
    if (found)
      matches->Add(new NMatch(position, position + 1, ngram.TokenCount()),
		   this);
  }
protected:
  RegExp* regexp_;
  string word_;
  hash_set<string, StringHash>* wordlist_;
};

class TagPattern : public WordPattern {
public:
  NPattern* clone() const { return new TagPattern;}
  virtual void Init(const Vlist& args) {
    WordPattern::Init(args);
    max_count_only_ = args.HasLabel(":max-count-only");
  }
  void PartialMatch(const Ngram& ngram, int position, NMatchVec* matches) {
    if (position >= ngram.Length() || ngram.TagSeqLen() == 0)
      return;
    int64 total = 0;
    if (max_count_only_) {
      int arg_max = 0;
      for (int i = 1; i < ngram.TagSeqLen(); ++i) {
	if (ngram.TagCount(i) > ngram.TagCount(arg_max))
	  arg_max = i;
      }
      const char* tag = ngram.Tag(arg_max, position);
      if (regexp_ != 0) {
	if (regexp_->exact_match(tag))
	  total += ngram.TagCount(arg_max);
      } else if (word_ == tag)
	total += ngram.TagCount(arg_max);
    } else {
      for (int i = 0; i < ngram.TagSeqLen(); ++i) {
	const char* tag = ngram.Tag(i, position);
	if (regexp_ != 0) {
	  if (regexp_->exact_match(tag))
	    total += ngram.TagCount(i);
	} else if (word_ == tag)
	  total += ngram.TagCount(i);
      }
    }
    if (total > 0)
      matches->Add(new NMatch(position, position + 1, total), this);
  }
protected:
  bool max_count_only_;
};

class WordSeqPattern : public NPattern {
public:
  WordSeqPattern() { trie_ = 0;}
  ~WordSeqPattern() {
    for (int i = 0; i < regexps_.size(); ++i)
      delete regexps_[i];
    delete trie_;
  }
  NPattern* clone() const { return new WordSeqPattern;}
  virtual void Init(const Vlist& args) {
    min_len_ = max_len_ = -1;
    if (args.str(1) == "=") {
      trie_ = new Trie;
      trie_->AddEntry(args.str(2));
      GetRange(trie_, &min_len_, &max_len_, 0);
    } else if (!args[1]->Atom() || args.str(1) == "~") {
      const Vlist* arglist = args[1];
      if (arglist->Atom())
	arglist = args[2];
      regexps_.resize(arglist->size());
      for (int i = 0; i < arglist->size(); ++i) {
	regexps_[i] = new RegExp(arglist->str(i).c_str());
      }
      min_len_ = max_len_ = regexps_.size();
    } else if (args.str(1) == "in") {
      trie_ = new Trie;
      for (int i = 2; i < args.size(); ++i) {
	if (args[i]->str().empty()) {
	  for (int j = 0; j < args[i]->size(); ++j) {
	    const string& s = args[i]->str(j);
	    if (!s.empty())
	      trie_->AddEntry(s);
	  }
	} else {
	  ifstream file(args[i]->str().c_str());
	  string line;
	  while (getline(file, line))
	    trie_->AddEntry(line);
	}
      }
      GetRange(trie_, &min_len_, &max_len_, 0);
    }
  }
  void GetRange(Trie* trie, int* min, int* max, int p) {
    if (trie == 0 || trie->is_end()) {
      if (*min < 0 || *min > p)
	*min = p;
      if (*max < 0 || *max < p)
	*max = p;
    }
    if (trie) {
      for (Trie::iterator it = trie->begin(); it != trie->end(); ++it) {
	GetRange(it->second, min, max, p + 1);
      }
    }
  }
  void LookUp(Trie* trie, const Ngram& ngram, int position, vector<int>* results) {
    if (trie->is_end())
      results->push_back(position);
    if (position == ngram.Length())
      return;
    string token = ngram.Token(position);
    map<string, Trie*>::iterator it = trie->find(token);
    if (it != trie->end())
      LookUp(it->second, ngram, position + 1, results);
  }
  void LookUpTag(Trie* trie, const Ngram& ngram, int tag_index, int position, vector<int>* results) {
    if (trie->is_end())
      results->push_back(position);
    if (position == ngram.Length())
      return;
    string tag = ngram.Tag(tag_index, position);
    map<string, Trie*>::iterator it = trie->find(tag);
    if (it != trie->end())
      LookUpTag(it->second, ngram, tag_index, position + 1, results);
  }
  void PartialMatch(const Ngram& ngram, int position, NMatchVec* matches) {
    if (trie_ == 0) {
      if (position + regexps_.size() > ngram.Length())
	return;
      for (int k = 0; k < regexps_.size(); ++k) {
	const char* word = ngram.Token(position + k);
	if (!regexps_[k]->exact_match(word))
	  return;
      }
      matches->Add(new NMatch(position, position + regexps_.size(), ngram.TokenCount()), this);
    } else {
      vector<int> results;
      LookUp(trie_, ngram, position, &results);
      for (int i = 0; i < results.size(); ++i) {
	matches->Add(new NMatch(position, results[i], ngram.TokenCount()), this);
      }
    }
  }
protected:
  vector<RegExp*> regexps_;
  Trie* trie_;
};

class TagSeqPattern : public WordSeqPattern {
public:
  NPattern* clone() const { return new TagSeqPattern;}
  void PartialMatch(const Ngram& ngram, int position, NMatchVec* matches) {
    if (trie_ == 0) {
      if (position + regexps_.size() > ngram.Length() || ngram.TagSeqLen() == 0)
	return;
      int64 total = 0;
      for (int i = 0; i < ngram.TagSeqLen(); ++i) {
	int matched = true;
	for (int k = 0; k < regexps_.size() && matched; ++k) {
	  const char* tag = ngram.Tag(i, position + k);
	  if (!regexps_[k]->exact_match(tag))
	    matched = false;
	}
	if (matched)
	  total += ngram.TagCount(i);
      }
      if (total > 0)
	matches->Add(new NMatch(position, position + regexps_.size(), total), this);
    } else {
      vector<int64> total;
      total.assign(max_len_ + 1, 0);
      for (int i = 0; i < ngram.TagSeqLen(); ++i) {
	vector<int> results;
	LookUpTag(trie_, ngram, i, position, &results);
	for (int k = 0; k < results.size(); ++k) {
	  total[results[k] - position] += ngram.TagCount(i);
	}
      }
      for (int i = 1; i < total.size(); ++i) {
	if (total[i] > 0)
	  matches->Add(new NMatch(position, position + i, total[i]), this);
      }
    }
  }
};


// Abstract superclass of all composite patterns
class CompPattern : public NPattern {
public:
  ~CompPattern() {
    for (int i = 0; i < subs_.size(); ++i) {
      delete subs_[i];
    }
  }
  virtual void Init(const Vlist& args) {
    assert(args.size() > 1);
    int len = 0;
    for (int i = 1; i < args.size(); ++i) {
      if (args[i]->Atom())
	break;
      ++len;
    }
    subs_.resize(len);
    for (int i = 0; i < len; ++i) {
      subs_[i] = Create(*args[i + 1]);
    }
  }
protected:
  vector<NPattern*> subs_;
};

class SeqPattern : public CompPattern {
public:
  NPattern* clone() const { return new SeqPattern;}
  virtual void Init(const Vlist& args) {
    CompPattern::Init(args);
    min_len_ = max_len_ = 0;
    for (int i = 0; i < subs_.size(); ++i) {
      min_len_ += subs_[i]->min_len();
      max_len_ += subs_[i]->max_len();
    }
  }
  void PartialMatchAux(const Ngram& ngram, int position, NMatchVec* matches,
		       int i) {
    NMatchVec first;
    subs_[i]->PartialMatch(ngram, position, &first);
//    cerr << "<first>" << endl;
//    first.Print(cerr);
//    cerr << "\n</first>" << endl;
    if (i + 1 == subs_.size()) {
      for (int j = 0; j < first.size(); ++j) {
	matches->Add(first[j]->Project(), this);
	first[j] = 0;
      }
    }
    else if (i < subs_.size()) {
      for (int k = 0; k < first.size(); ++k) {
	NMatchVec rest;
	PartialMatchAux(ngram, first[k]->to, &rest, i + 1);
//	cerr << "<rest>" << endl;
//	rest.Print(cerr);
//	cerr << "\n</rest>" << endl;
	for (int j = 0; j < rest.size(); ++j) {
	  NMatch* f = first[k];
	  if (j == rest.size() - 1) {
	    first[k] = 0; // for the last tail match, we don't need to
			  // copy the head
	  } else {
	    f = first[k]->DeepCopy();
	  }
	  rest[j]->Prepend(f);
	  matches->Add(rest[j], this);
	  rest[j] = 0;
	}
      }
    }
  }
  virtual void PartialMatch(const Ngram& ngram, int position,
			    NMatchVec* matches) {
    if (position >= ngram.Length())
      return;
    PartialMatchAux(ngram, position, matches, 0);
  }
};

class AndPattern : public CompPattern {
public:
  NPattern* clone() const { return new AndPattern;}
  virtual void Init(const Vlist& args) {
    CompPattern::Init(args);
    min_len_ = max_len_ = 0;
    for (int i = 0; i < subs_.size(); ++i) {
      if (i == 0) {
	min_len_ = subs_[i]->min_len();
	max_len_ = subs_[i]->max_len();
      } else {
	if (min_len_ < subs_[i]->min_len())
	  min_len_ = subs_[i]->min_len();
	if (max_len_ > subs_[i]->max_len())
	  max_len_ = subs_[i]->max_len();
      }
    }
  }
  virtual void PartialMatch(const Ngram& ngram, int position,
			    NMatchVec* matches) {
    vector<NMatchVec> results(subs_.size());
    for (int i = 0; i < subs_.size(); ++i) {
      subs_[i]->PartialMatch(ngram, position, &results[i]);
      if (results[i].empty())
        return;
    }
    int last = subs_.size() - 1;
    int last_size = results[last].size();
    for (int i = 0; i < last_size; ++i) {
      NMatch* m = results[last][i];
      bool found_in_all = true;
      for (int k = 0; found_in_all && k < last; ++k) {
	bool found = false;
	int k_size = results[k].size();
	for (int j = 0; !found && j < k_size; ++j) {
	  NMatch* match = results[k][j];
	  if (m->ShallowEqualTo(*match)) {
	    if (m->count > match->count)
	      m->count = match->count;
	    found = true;
	  }
	}
	if (!found)
	  found_in_all = false;
      }
      if (found_in_all) {
	matches->Add(m->Project(), this);
	results[last][i] = 0;
      }
    }
  }
};

class OrPattern : public CompPattern {
public:
  NPattern* clone() const { return new OrPattern;}
  virtual void Init(const Vlist& args) {
    CompPattern::Init(args);
    min_len_ = max_len_ = 0;
    for (int i = 0; i < subs_.size(); ++i) {
      if (i == 0) {
	min_len_ = subs_[i]->min_len();
	max_len_ = subs_[i]->max_len();
      } else {
	if (min_len_ > subs_[i]->min_len())
	  min_len_ = subs_[i]->min_len();
	if (max_len_ < subs_[i]->max_len())
	  max_len_ = subs_[i]->max_len();
      }
    }
  }
  virtual void PartialMatch(const Ngram& ngram, int position,
			    NMatchVec* matches) {
    vector<NMatchVec> results(subs_.size());
    for (int i = 0; i < subs_.size(); ++i) {
      subs_[i]->PartialMatch(ngram, position, &results[i]);
    }
    NMatchVec uniq;
    for (int i = 0; i < results.size(); ++i) {
      for (int k = 0; k < results[i].size(); ++k) {
	NMatch* m = results[i][k];
	bool found = false;
	for (int j = 0; !found && j < uniq.size(); ++j) {
	  if (m->fr == uniq[j]->fr && m->to == uniq[j]->to)
	    found = true;
	}
	if (!found) {
	  uniq.Add(m, m->pat);
	  results[i][k] = 0;
	}
      }
    }
    for (int i = 0; i < uniq.size(); ++i) {
      matches->Add(uniq[i]->Project(), this);
      uniq[i] = 0;
    }
  }
};

class NotPattern : public NPattern {
public:
  NotPattern() { sub_ = 0;}
  NPattern* clone() const { return new NotPattern;}
  ~NotPattern() {
    delete sub_;
  }
  virtual void Init(const Vlist& args) {
    assert(args.size() >= 2);
    sub_ = Create(*args[1]);
    min_len_ = sub_->min_len();
    max_len_ = sub_->max_len();
    if (min_len_ != 1 || max_len_ != 1) {
      cerr << "The 'not' pattern can only take patterns that matches a single token as the subpattern"; 
      exit(1);
    }
  }
  virtual void PartialMatch(const Ngram& ngram, int position,
			    NMatchVec* matches) {
    NMatchVec results;
    sub_->PartialMatch(ngram, position, &results);
    if (results.empty())
      matches->Add(new NMatch(position, position + 1, ngram.TokenCount()), this);
  }
protected:
  NPattern* sub_;
};

class PlusPattern : public NPattern {
public:
  PlusPattern() { min_ = 1; max_ = 5; sub_ = 0;}
  NPattern* clone() const { return new PlusPattern;}
  ~PlusPattern() {
    delete sub_;
  }
  virtual void Init(const Vlist& args) {
    assert(args.size() >= 2);
    sub_ = Create(*args[1]);
    const Vlist* min = args.Get(":min");
    if (min)
      min_ = atoi(min->str().c_str());
    const Vlist* max = args.Get(":max");
    if (max)
      max_ = atoi(max->str().c_str());
    min_len_ = min_ * sub_->min_len();
    max_len_ = max_ * sub_->max_len();
  }
  virtual void PartialMatch(const Ngram& ngram, int position,
			    NMatchVec* matches) {
    if (position >= ngram.Length())
      return;
    vector<bool> found(ngram.Length() + 1, false);
    PartialMatchAux(ngram, position, matches, &found, min_, max_);
  }
  virtual void PartialMatchAux(const Ngram& ngram, int position,
			       NMatchVec* matches, vector<bool>* found,
			       int range_fr, int range_to) {
    if (position >= ngram.Length())
      return;
    NMatchVec submatches;
    sub_->PartialMatch(ngram, position, &submatches);
    for (int i = 0; i < submatches.size(); ++i) {
      if ((*found)[submatches[i]->to])
	continue;
      if (range_fr <= 1 && range_to >= 1)
	matches->Add(submatches[i]->DeepCopy()->Project(), this);
      (*found)[submatches[i]->to] = true; 
      if (submatches[i]->to == ngram.Length() || range_to == 1)
	continue;
      NMatchVec more;
      PartialMatchAux(ngram, submatches[i]->to, &more, found,
		      range_fr - 1, range_to - 1);
      for (int k = 0; k < more.size(); ++k) {
	more[k]->Prepend(submatches[i]->DeepCopy());
	matches->Add(more[k], this);
	more[k] = 0;
      }
    }
  }
protected:
  NPattern* sub_;
  int min_; // inclusive
  int max_; // inclusive
};

class StarPattern : public PlusPattern {
public:
  NPattern* clone() const { return new StarPattern;}
  virtual void Init(const Vlist& args) {
    PlusPattern::Init(args);
    min_len_ = 0;
  }
  virtual void PartialMatch(const Ngram& ngram, int position, NMatchVec* matches) {
    if (position < ngram.Length()) {
      vector<bool> found(ngram.Length() + 1, false);
      PartialMatchAux(ngram, position, matches, &found,
		      min_, max_);
    }
    matches->Add(new NMatch(position, position, -1), this);
  }
};

class OptionalPattern : public PlusPattern {
public:
  NPattern* clone() const { return new OptionalPattern;}
  virtual void Init(const Vlist& args) {
    PlusPattern::Init(args);
    min_len_ = 0;
    max_len_ = sub_->max_len();
  }
  virtual void PartialMatch(const Ngram& ngram, int position, NMatchVec* matches) {
    if (position < ngram.Length()) {
      NMatchVec submatches;
      sub_->PartialMatch(ngram, position, &submatches);
      for (int i = 0; i < submatches.size(); ++i) {
	matches->Add(submatches[i]->Project(), this);
	submatches[i] = 0;
      }
    }
    matches->Add(new NMatch(position, position, -1), this);
  }
};

void NPattern::RegisterProto(const string& type, NPattern* proto) {
  assert(protos.find(type) == protos.end());
  protos[type] = proto;
}

void NMatch::ExtractKeyValPair(pair<const NMatch*, const NMatch*>* pair,
			       const string& key) const {
  if (pat) {
    if (pat->Name() == key && pair->first == 0)
      pair->first = this;
    else if (pat->Name() != "" && pair->second == 0)
      pair->second = this;
  }
  if (pair->first && pair->second)
    return;
  for (int i = size() - 1; i >= 0; --i)
    (*this)[i]->ExtractKeyValPair(pair, key);
}

void NMatch::NamedComponents(vector<const NMatch*>* named_matches) const {
  if (pat && !pat->Name().empty())
    named_matches->push_back(this);
  for (int i = size() - 1; i >= 0; --i)
    (*this)[i]->NamedComponents(named_matches);
}

bool NPattern::Match(const Ngram& ngram, NMatchVec* matches) {
  NMatchVec m;
  PartialMatch(ngram, 0, &m);
  bool found = false;
  for (int i = 0; i < m.size(); ++i) {
    if (m[i]->to == ngram.Length()) {
      if (matches)
        matches->Add(m[i], this);
      m[i] = 0;
      found = true;
    }
  }
  return found;
}

void NgramInfoCollector::CollectAll(PrefixIterator* it) {
  Start();
  Ngram ngram;
  int count = 0;
  if (max_match_ == 1) {
    string r = it->Search(prefix_query_);
    ngram.Init(r);
    CollectInfo(ngram);
  } else {
    int prefix_len = Ngram::Length(prefix_query_);
    for (it->Seek(prefix_query_); !it->Done(); it->Next()) {
      int len = Ngram::Length(it->Current());
      if (len > max_len_ && max_len_ <= prefix_len)
	break;
      if (len < min_len_ || len > max_len_)
	continue;
      ngram.Init(it->Current());
      if (CollectInfo(ngram) && max_match_ && ++count >= max_match_)
	break;
    }
  }
  Flush();
}

void NgramInfoCollector::Init(const Vlist& args) {
  assert(args.size() > 1);
  pat_ = NPattern::Create(*args[1]);
  min_len_ = pat_->min_len();
  max_len_ = pat_->max_len();
  prefix_query_ = args.GetAtom(":prefix-query");
  string max = args.GetAtom(":max-match");
  if (max != "")
    max_match_ = atoi(max.c_str());
  if (args.HasLabel(":log-count"))
    log_count_ = true;
  if (args.HasLabel(":uniq-count"))
      uniq_count_ = true;
  if (args.HasLabel(":allow-partial-match"))
      allow_partial_match_ = true;
}

class PrintNgram : public NgramInfoCollector {
public:
  NgramInfoCollector* clone() const { return new PrintNgram;}
  bool CollectInfo(const Ngram& ngram) {
    if (allow_partial_match_) {
      bool found = false;
      for (int i = 0; i < ngram.Length(); ++i) {
        NMatchVec matches;
        pat_->PartialMatch(ngram, i, &matches);
        if (!matches.empty()) {
          printer()->Print(ngram.orig());
          found = true;
        }
      }
      return found;
    } else {
      NMatchVec matches;
      if (pat_->Match(ngram, &matches)) {
        printer()->Print(ngram.orig());
        return true;
      } else
        return false;
    }
  }
};

class KeyValInfo : public NgramInfoCollector {
public:
  virtual void CollectFromOne(const Ngram& ngram, const NMatch* match,
			      int64 count) {
    pair<const NMatch*, const NMatch*> pair(0, 0);
    match->ExtractKeyValPair(&pair, key_);
    if (pair.first && pair.second) {
      string key, val;
      if (use_name_as_key_)
	key = pair.first->pat->Name();
      else
	key = ngram.Concat(pair.first->fr, pair.first->to);
      if (val_inst_)
	val = ngram.Concat(pair.second->fr, pair.second->to);
      else
	val = pair.second->pat->Name();
      counts_[key][val] += count;
    }
  }
  void Start() { counts_.clear();}
  bool CollectInfo(const Ngram& ngram) {
    if (allow_partial_match_) {
      bool found = false;
      for (int i = 0; i < ngram.Length(); ++i) {
        NMatchVec matches;
        pat_->PartialMatch(ngram, i, &matches);
        if (!matches.empty()) {
          for (int j = 0; j < matches.size(); ++j) {
            CollectFromOne(ngram, matches[j], GetCount(matches[j]->count));
          }
          found = true;
        }
      }
      return found;
    } else {
      NMatchVec matches;
      pat_->Match(ngram, &matches);
      for (int i = 0; i < matches.size(); ++i) {
        CollectFromOne(ngram, matches[i], GetCount(matches[i]->count));
      }
      return !matches.empty();
    }
  }
  void Flush() {
    if (counts_.empty()) {
      cout << "<EMPTY>" << endl;
    } else {
      for (hash_map<string, map<string, int64>, StringHash>::iterator
	     i = counts_.begin(); i != counts_.end(); ++i) {
	cout << i->first;
	for (map<string, int64>::iterator j = i->second.begin();
	     j != i->second.end(); ++j) {
	  cout << '\t' << j->first << '\t' << j->second;
	}
	cout << endl;
      }
    }
  }
  virtual void Init(const Vlist& args) {
    NgramInfoCollector::Init(args);
    key_ = args.GetAtom(":key");
    assert(key_ != "");
    if (args.HasLabel(":val-inst"))
      val_inst_ = true;
    if (args.HasLabel(":use-name-as-key"))
      use_name_as_key_ = true;
  };
  KeyValInfo() {
    val_inst_ = false;
    log_count_ = false;
    use_name_as_key_ = false;
  }
  NgramInfoCollector* clone() const { return new KeyValInfo;}
protected:
  hash_map<string, map<string, int64>, StringHash> counts_;
  string key_;
  bool val_inst_;
  bool log_count_;
  bool use_name_as_key_;
};

class BatchNgramInfoCollector : public NgramInfoCollector {
public:
  void Start() {
    for (int i = 0; i < collectors_.size(); ++i) {
      collectors_[i]->Start();
      collectors_[i]->set_printer(printer_);
    }
  }
  bool CollectInfo(const Ngram& ngram) {
    bool found = false;
    for (int i = 0; i < collectors_.size(); ++i) {
      found |= collectors_[i]->CollectInfo(ngram);
    }
    return found;
  }
  void Flush() {
    for (int i = 0; i < collectors_.size(); ++i) {
      collectors_[i]->Flush();
      collectors_[i]->set_printer(0);
    }
  }
  virtual void Init(const Vlist& args) {
    for (int i = 1; i < args.size(); ++i) {
      if (args[i]->Atom())
	break;
      NgramInfoCollector* col = NgramInfoCollector::Create(*args[i]);
      collectors_.push_back(col);
      if (i == 1) {
	min_len_ = col->min_len();
	max_len_ = col->max_len();
      } else {
	if (min_len_ > col->min_len())
	  min_len_ = col->min_len();
	if (max_len_ < col->max_len())
	  max_len_ = col->max_len();
      }
    }
  };
  NgramInfoCollector* clone() const { return new BatchNgramInfoCollector;}
protected:
  vector<NgramInfoCollector*> collectors_;
};


class CountNamed : public NgramInfoCollector {
public:
  CountNamed() {
    output_separator_ = "\t";
  }
  const NMatch* FindMatch(const vector<string>& vars,
			  const vector<const NMatch*> named_matches) {
    int len = named_matches.size();
    for (int i = 0; i < vars.size(); ++i) {
      const string& v = vars[i];
      for (int j = 0; j < len; ++j) {
	if (v == named_matches[j]->pat->Name())
	  return named_matches[j];
      }
    }
    return 0;
  }
  virtual void CollectFromOne(const Ngram& ngram, const NMatch* match,
			      int64 count) {
    vector<const NMatch*> named_matches;
    match->NamedComponents(&named_matches);
    string key;
    if (formats_.empty()) {
      for (int i = 0; i < named_matches.size(); ++i) {
	key = named_matches[i]->pat->Name();
	counts_[key] += count;
      }
      return;
    }
    for (int i = 0; i < formats_.size(); ++i) {
      Format& f = formats_[i];
      key += f.prefix;
      if (f.vars.empty())
	continue;
      const NMatch* m = FindMatch(f.vars, named_matches);
      if (m == 0)
	return;
      if (f.use_name)
	key += m->pat->Name();
      else
	key += ngram.Concat(m->fr, m->to);
    }
    counts_[key] += count;    
  }
  virtual void Init(const Vlist& args) {
    NgramInfoCollector::Init(args);
    string s = args.GetAtom(":format");
    // Parse s
    vector<string> fields;
    SplitStringUsing(s, "]", &fields);
    formats_.resize(fields.size());
    for (int i = 0; i < fields.size(); ++i) {
      string field = fields[i];
      int p = field.find_last_of('[');
      if (p == string::npos) {
	formats_[i].prefix = field;
	continue;
      }
      SplitStringUsing(field.substr(p + 1), "|", &formats_[i].vars);
      if (p > 0 && field[p - 1] == '$') {
	formats_[i].use_name = false;
	--p;
      }
      formats_[i].prefix = field.substr(0, p);
    }
  };
  void Start() { counts_.clear();}
  bool CollectInfo(const Ngram& ngram) {
    if (allow_partial_match_) {
      bool found = false;
      for (int i = 0; i < ngram.Length(); ++i) {
        NMatchVec matches;
        pat_->PartialMatch(ngram, i, &matches);
        if (!matches.empty()) {
          for (int j = 0; j < matches.size(); ++j) {
            CollectFromOne(ngram, matches[j], GetCount(matches[j]->count));
          }
          found = true;
        }
      }
      return found;
    } else {
      NMatchVec matches;
      pat_->Match(ngram, &matches);
      for (int i = 0; i < matches.size(); ++i) {
        CollectFromOne(ngram, matches[i], GetCount(matches[i]->count));
      }
      return !matches.empty();
    }
  }
  void Flush() {
    for (hash_map<string, int64, StringHash>::iterator
	     i = counts_.begin(); i != counts_.end(); ++i) {
      printer()->Print(StringPrintf("%s\t%lld%s", i->first.c_str(), i->second,
                                    output_separator_.c_str()));
    }
  }
  NgramInfoCollector* clone() const { return new CountNamed;}
protected:
  struct Format {
    string prefix;
    vector<string> vars;
    bool use_name;
    Format() {
      use_name = true;
    }
  };
  vector<Format> formats_;
  hash_map<string, int64, StringHash> counts_;
  string output_separator_;
};

class CountMatch : public CountNamed {
public:
  NgramInfoCollector* clone() const { return new CountMatch;}
  CountMatch() {
    output_separator_ = "\n";
  }
};

void NgramInfoCollector::RegisterProto(const string& type,
				       NgramInfoCollector* proto) {
  assert(protos.find(type) == protos.end());
  protos[type] = proto;
}

NgramInfoCollector* NgramInfoCollector::Create(const string& args) {
  Vlist v;
  v.Read(args);
  return NgramInfoCollector::Create(v);
}

NgramInfoCollector* NgramInfoCollector::Create(const Vlist& args) {
  const NgramInfoCollector* proto = protos[args[0]->str()];
  if (proto == 0)
    cerr << "Unknown NgramInfoCollector type: " << args[0]->str() << endl;
  assert(proto != 0);
  NgramInfoCollector* p = proto->clone();
  p->Init(args);
  return p;
}

hash_map<string, NPattern*, StringHash> NPattern::protos;
hash_map<string, NgramInfoCollector*, StringHash> NgramInfoCollector::protos;

void NPattern::InitProtos() {
  static bool done = false;
  if (done)
    return;
  done = true;

  // Atomic patterns that match a single token (position).
  NPattern::RegisterProto("word", new WordPattern);
  NPattern::RegisterProto("tag", new TagPattern);
  NPattern::RegisterProto("tag-seq", new TagSeqPattern);
  NPattern::RegisterProto("word-seq", new WordSeqPattern);
  NPattern::RegisterProto("t", new TPattern);
  NPattern::RegisterProto("><", new DividerPattern);

  // Single component patterns
  NPattern::RegisterProto("+", new PlusPattern);
  NPattern::RegisterProto("*", new StarPattern);
  NPattern::RegisterProto("?", new OptionalPattern);
  NPattern::RegisterProto("not", new NotPattern);

  // Composite patterns
  NPattern::RegisterProto("seq", new SeqPattern);
  NPattern::RegisterProto("and", new AndPattern);
  NPattern::RegisterProto("or", new OrPattern);

  // Patterns on the concatenated string of the complete token
  // sequence or tag sequence.
  // Example:
  // (grep "the .* dog") matches any token sequence that begins with
  // 'the' and ends with 'dog'.
  NPattern::RegisterProto("grep", new GrepPattern);
  //    NPattern::RegisterProto("grep-tag", new GrepTagPattern);

  NgramInfoCollector::RegisterProto("print-ngram", new PrintNgram);
  NgramInfoCollector::RegisterProto("count-key-val", new KeyValInfo);
  NgramInfoCollector::RegisterProto("extractor-set", new BatchNgramInfoCollector);
  NgramInfoCollector::RegisterProto("count-named", new CountMatch);
  NgramInfoCollector::RegisterProto("count", new CountMatch);
}

struct Initializer {
  Initializer() {
    NPattern::InitProtos();
  }
};

Initializer init;
