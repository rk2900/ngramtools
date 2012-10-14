#!/bin/sh

ngram_files=example_data/sample.txt
partition_bounds=example_data/index.txt
stop_words=example_data/stop_words.txt
prefix=example_data/ngm

for f in $ngram_files
do
    echo $f
    ./rotate_ngrams --bounds=$partition_bounds --stop_words=$stop_words --prefix=$prefix < $f
done

for f in $prefix.???
do
    echo sort $f
    mv $f $f.bak
    sort $f.bak > $f
    rm $f.bak
done

rm -f $prefix.index

for f in $prefix.???
do
    echo index $f
    ./index_ngrams --index_chunk_size=1000000 < $f >> $prefix.index
done
