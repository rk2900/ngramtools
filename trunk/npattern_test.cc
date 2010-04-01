#include <sstream>
#include <iostream>

#include "npattern.h"

using namespace std;

int main() {
  NPattern* pat = NPattern::Create("seq (and (+ (t) :min 2 :max 3) (seq (t) (t) (* (word = and)))) (word = token) (word ~ s.*e) (* (t)))");
  NPattern* pat2 = NPattern::Create("seq (tag = DT) (* (or (tag ~ [NJ].* :name individual-mod) (tag = VBZ)) :name mod) (word = ex-wife)");
  NPattern* pat3 = NPattern::Create("seq (word = ex-wife) (tag = CC) (and (t :name T) (or (word = his :name M) (word = her :name F) (word = my :name O)))");

  Ngram a, b, c, d, e;
  a.Init("token space >< and surrogate ,	10	T ,|JJ|CC|JJ|NN|2 ,|NN|CC|JJ|NN|8");
  b.Init("token space : Grammatical	18	T JJ|NN|:|JJ|18");
  c.Init("token space >< and	10	T CC|JJ|NN|2 CC|JJ|NN|8");
  d.Init("ex-wife >< s ’ appellant the	17	T DT|NN|NN|VBZ|NN|17");
  e.Init("ex-wife and his	594	T NN|CC|PRP$|594");
  hash_map<string, StrIntMap, StringHash> matched;
  pat3->CountMatched(e, &matched);
  NMatchVec ma, mb, mc, md;
  pat2->Match(d, &md);
  md.Print(cerr);
  StrIntMap counts;
  pat2->Count(d, &counts);
  for (StrIntMap::iterator t = counts.begin();
       t != counts.end(); ++t) {
    cerr << t->first.c_str() << '\t' << t->second << endl;
  }
  pat2->CountMatched(d, &matched);
  for (hash_map<string, StrIntMap, StringHash>::iterator it = matched.begin();
       it != matched.end(); ++it) {
    cerr << it->first << ": ";
    for (StrIntMap::iterator t = it->second.begin();
         t != it->second.end(); ++t) {
      cerr << t->first.c_str() << ' ' << t->second << ", ";
    }
    cerr << endl;
  }
  pat->Match(a, &ma);
  ma.Print(cerr);
  pat->Match(b, &mb);
  mb.Print(cerr);
  pat->Match(c, &mc);
  mc.Print(cerr);
  delete pat;
  delete pat2;
  delete pat3;
}
