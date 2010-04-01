This directory contains programs for n-gram search and pattern
matching.

There are three executables:

./ngram_server INDEX_FILE [PORT]
  Start an server for looking up the ngrams corresponding to
  INDEX_FILE. The default port number is 6700
  For example:
  ./ngram_server /export/ws09/dlin/GoogleV2/rotated.index 33333

./search_prefix [INDEX_FILE|HOSTNAME:PORT] PREFIX [EXTRACTOR]
   Search the PREFIX in the ngram data collection. If an EXTRACTOR is
   give, it will be used to process the results. The ngrams can be
   looked up eithers with the INDEX_FILE or from an ngram server at
   the specified host:port.
   For example:
   ./search_prefix localhost:33333 "time flies"

./batch_counting EXTRACTOR or ./batch_counting -file FILE
   The batch_counting program can be run with Hadoop. The file vbn_vbd.sh
   contains an exmaple.

The EXTRACTORs for the above executables can be the following:
  (count PATTERN :format FORMAT [:max-match N] [:prefix-query PHRASE])
    Whenever an n-ngram is found to match the pattern, we aggregate
    the count of the string with the given format. For example
    head -1000000 /export/ws09/dlin/GoogleV2/rotated-878.txt | ./batch_counting '
        (count (seq (+ (tag ~ [NJ].*) :name noun)
                    (or (word = who :name animate)
                        (word = which :name inanimate)))
           :format $[noun]:[animate|inanimate])'

    The special characters in a FORMAT string include '[', ']', '|',
    and '$'. The square brackets should contain one or more (separated
    by  '|') names of subpatterns. If a '$' precedes the '[', the
    matching subsequence in the ngram is used to construct the counted
    string. Otherwise the subpattern name is used.

  (print-ngram PATTERN [:max-match N] [:prefix-query PHRASE])
     Print the ngrams that matche the pattern.

  (count-key-val PATTERN key: NAME [:val-inst] [:log-count] [:max-match N] [:prefix-query PHRASE]):
     Output the counts of key-value pairs defined in the pattern. The
     key is whatever matched the component of the PATTERN with the
     given NAME. The corresponding value is whatever matches another
     named component (with a different name). Therefore there must be
     other named components in the PATTERN in addition to the key. For
     example, the following extractor counts the animate and inanimate
     features for each noun/adjective sequence.

     (count-key-val (seq (+ (tag ~ [NJ].*) :name noun)
                         (or (word = who :name animate)
                             (word = which :name inanimate)))
                    :key noun)
 		    
     For another example, the following extractor counts the determiners
     for noun phrases. 
     (count-key-val (seq (or (tag = IN :name none)
                             (and (tag = DT)
                                  (or (word in (the The this This) :name definite)
                                      (word in (a A an An) :name indefinite))))
                         (+ (tag ~ [NJ].*) :name noun)
                         (tag ~ "(:|,|IN|V.*)" :max-count-only))
                    :key noun)

    By default, the names of component patterns are used as values
    (e.g., definite and indefinite in the above pattern). If the
    option :val-inst is given, the token sequences that matched the
    component pattern are treated as the values begin counted.

  (count-named PATTERN [:max-match N] [:prefix-query PHRASE])
    Outputs the total counts of all ngrams that matched the named
    components in the pattern.

  (extractor-set EXTRACTOR EXTRACTOR ...EXTRACTOR)
    

N-gram Patterns

Atomic Patterns:

(word = WORD) or (word ~ REGEXP) or (word in LIST)
  matches a single token that is equal to the word or matches the
  regular expression. Here, LIST can either be a LISP-like list, e.g.,
  (one two three), or the name of a text file where each line is an
  element in the list.

(tag = TAG [:max-count-only]) or (tag ~ REGEXP [:max-count-only]) or
(tag in LIST [:max-count-only])
  matches a single tag that is equal to the word or matches the
  regular expression. When the flag :max-count-only is present, only
  the most frequent tag sequence is considered during the match.


(tag-seq in FILE)
(tag-seq in ("SPACE SEPARATED TAGS" ... "SPACE SEPARATED TAGS")
(tag-seq (REGEXP ... REGEXP))
(tag-seq = "SPACE SEPARATED TAGS")
(tag-seq ~ "SPACE SEPARATED REGEXPS")
(tag-seq (REGEXP ... REGEXP))
  matches a single POS tag sequence where each component statisfies
  the corresponding regular expression or is equal to (one of) the
  given tag sequence. If a file name is given, each line in the file
  is assumed to be a space separated sequence.

(word-seq in FILE)
(word-seq in ("SPACE SEPARATED WORDS" ... "SPACE SEPARATED WORDS")
(word-seq (REGEXP ... REGEXP))
(word-seq = "SPACE SEPARATED WORDS")
(word-seq ~ "SPACE SEPARATED REGEXPS")
(word-seq (REGEXP ... REGEXP))
  matches a sequence of words where each component statisfies
  the corresponding regular expression or is equal to (one of) the
  given word sequence. If a file name is given, each line in the file
  is assumed to be a space separated sequence.

(t) matches any token

(><)
  matches the token immediately after the divider. In the rotated
  n-gram files, this is always the first token in a line.

Single-component Patterns
(+ PATTERN [:min N1] [:max N2])
  matches N1 to N2 (both inclusive) subsequences that matches the
  PATTERN (N1 >= 1)

(* PATTERN [:max N])
  matches 0 to N (inclusive) subsequences that matches the PATTERN

Multi-component Patterns
(seq PATTERN1 PATTERN2 ...... PATTERNn)
  matches a sequence of tokens

(and PATTERN1 PATTERN2 ...... PATTERNn)
  matches a sequence if the sequence matches all the patterns PATTERN1
  PATTERN2 ...... PATTERNn. 

(or  PATTERN1 PATTERN2 ...... PATTERNn)
  matches a sequence if the sequence matches any of the patterns PATTERN1
  PATTERN2 ...... PATTERNn. 

Whole N-gram Patterns
(grep REGEXP)
  match a whole n-ngram if the concatenated string of the n-gram (joined
  with spaces) matches the regular expression.

(grep-tag REGEXP)
  match a whole n-gram if the concatenated tags of the n-gram (joined
  with '|') matches the regular expression.


