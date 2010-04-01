/**
 * @author      Dekang Lin <lindek@gmail.com>
 * @author      ... add your name here if you make changes to this code
 * @license     http://www.gnu.org/copyleft/gpl.html
 *              GNU GENERAL PUBLIC LICENSE version 2 or later
 */

#ifndef PREFIX_ITERATOR
#define PREFIX_ITERATOR

#include "string_utils.h"

// We assume that an ordered set lines are stored in a set of files
// (the ordering is global across all files). PrefixIterator is an
// abstract class for searching for lines with a given prefix. Suppose
// itor is an instance of a subclass of PrefixIterator. The
// following loop prints all the lines with the given prefix.
// 
//  for (itor.Seek(line); !itor.Done(); itor.Next()) {
//    cout << itor.Current() << endl;
//  }
//
class PrefixIterator {
 public:
  virtual ~PrefixIterator() {}

  // Initialize the iterator with an index file. The ngram files are
  // assumed in the same directory as the index file
  virtual void Init(const string& index_file) = 0;

  virtual string Search(const string& prefix) {
    string result;
    Seek(prefix);
    if (!Done())
      result = Current();
    return result;
  }

  // Get the first matching ngram
  virtual void Seek(const string& prefix) = 0;

  // Move to the next matching ngram
  virtual void Next() = 0;

  // Return true if there is no more matching ngrams
  virtual bool Done() const = 0;

  // Return the current matching ngram
  virtual const string& Current() const = 0;
};

#endif
