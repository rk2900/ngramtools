/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#ifndef NGRAM_ITERATOR
#define NGRAM_ITERATOR

#include "prefix_iterator.h"

class NgramIterator : public PrefixIterator {
 public:
  NgramIterator() { impl_ = 0;}
  ~NgramIterator();

  // Initialize the iterator with an index file. The ngram files are
  // assumed in the same directory as the index file
  virtual void Init(const string& index_file);

  // Get the first matching ngram
  virtual void Seek(const string& key);

  // Move to the next matching ngram
  virtual void Next();

  // Return true if there is no more matching ngrams
  virtual bool Done() const;

  // Return the current matching ngram
  virtual const string& Current() const;
 protected:
  class Impl;
  Impl* impl_;
};

#endif
