/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#ifndef CLIENT_ITERATOR
#define CLIENT_ITERATOR

#include "prefix_iterator.h"

class ClientIterator : public PrefixIterator {
 public:
  ClientIterator() { impl_ = 0;}
  ~ClientIterator();

  virtual void Init(const string& server_and_port);

  virtual string Search(const string& prefix);

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
