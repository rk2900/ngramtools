#include <iostream>

#include "ngram.h"

using namespace std;

int main() {
  Ngram a, b;
  a.Init("token space >< and surrogate ,	10	T ,|JJ|CC|JJ|NN|2 ,|NN|CC|JJ|NN|8");
  b.Init("token space : Grammatical	18	T JJ|NN|:|JJ|18");
  a.Print(cout);
  b.Print(cout);
}
