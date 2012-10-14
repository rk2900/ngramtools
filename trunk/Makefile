CFILES=sock.cc ngram_iterator.cc string_utils.cc vlist.cc npattern.cc ngram.cc regexp.cc regulare.cc client_iterator.cc
bin=batch_counting search_prefix index_ngrams sum_reducer batch_retrieval sum_reducer batch_retrieval search rotate_ngrams

%.o:	%.cc
	g++ -c -o $@ $(CFLAGS) -DWS09 $<

CFLAGS=-g -std=c++0x

all:	${bin}


libngrams.a:	${CFILES:.cc=.o}
	@${AR} ${ARFLAGS} $@ ${CFILES:.cc=.o}
	ranlib $@

sum_reducer:	sum_reducer.o libngrams.a
	g++ $(CFLAGS) sum_reducer.o -L. -lngrams -o $@

batch_counting:	batch_counting.o libngrams.a
	g++ $(CFLAGS) batch_counting.o -L. -lngrams -o $@

rotate_ngrams:	rotate_ngrams.o libngrams.a
	g++ $(CFLAGS) rotate_ngrams.o -L. -lngrams -o $@

batch_retrieval:	batch_retrieval.o libngrams.a
	g++ $(CFLAGS) batch_retrieval.o -L. -lngrams -o $@

search_prefix:	search_prefix_main.o libngrams.a
	g++ $(CFLAGS) search_prefix_main.o -L. -lngrams -o $@

search:	search_main.o libngrams.a
	g++ $(CFLAGS) search_main.o -L. -lngrams -o $@

ngram_server:	ngram_server.o libngrams.a
	g++ $(CFLAGS) ngram_server.o -L. -lngrams -o $@

ngram_server_test:	ngram_server_test.o libngrams.a
	g++ $(CFLAGS) ngram_server_test.o -L. -lngrams -o $@

index_ngrams:	index_ngrams.o
	g++ $(CFLAGS) index_ngrams.o string_utils.o -o $@

npattern_test: npattern_test.o libngrams.a
	g++ $(CFLAGS) npattern_test.o -L. -lngrams -o $@

vlist_test: vlist.o vlist_test.o
	g++ $(CFLAGS)  vlist.o vlist_test.o -o $@

ngram_test: ngram.o ngram_test.o  string_utils.o 
	g++ $(CFLAGS)  ngram.o ngram_test.o string_utils.o -o $@

clean:
	rm -f *.o *.a *.exe npattern_test vlist_test ngram_test ${bin}




