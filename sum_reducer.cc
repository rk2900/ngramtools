#include "reducer_input.h"
#include "string_utils.h"

static bool CmpStrIntMapIterator(StrIntMap::iterator i1, StrIntMap::iterator i2) {
  return i1->first < i2->first;
}

class SumReducer {
public:
  void Reduce(ReducerInput* input) {
    StrIntMap counts;
    for (; !input->Done(); input->Next()) {
      vector<string> fields;
      SplitStringUsing(input->value(), "\t", &fields);
      int64 count = 1;
      if (fields.empty())
	continue;
      int len = fields.size();
      for (int i = 0; i < len; i += 2) {
	string key;
	if (i + 1 < len) {
	  key = fields[i];
	  count = atoll(fields[i + 1].c_str());
	} else 
	  count = atoll(fields[i].c_str());
	counts[key] += count;
      }
    }
    vector<StrIntMap::iterator> vec;
    for (StrIntMap::iterator it = counts.begin(); it != counts.end(); ++it) {
      vec.push_back(it);
    }
    sort(vec.begin(), vec.end(), CmpStrIntMapIterator);
    cout << input->key();
    for (int i = 0; i < vec.size(); ++i) {
      if (vec[i]->first.empty())
	cout << '\t' << vec[i]->second;
      else
	cout << '\t' << vec[i]->first << '\t' << vec[i]->second;
    }
    cout << endl;
  }
};

int main() {
  SumReducer r;
  for (ReducerInput input; !input.AllDone(); input.UnsetDone()) {
    r.Reduce(&input);
  }
}
