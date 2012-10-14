hadoopdir=/home/dlin/hadoop-0.17.2
#inputs=/user/sbergsma/GoogleV2_Unrotated/part-000??
inputs=/user/dlin/GoogleV2Sharded

file=`basename $1 .txt`
outputs=$file
dir=/home/dlin/ngrams

$hadoopdir/bin/hadoop fs -rmr $outputs

$hadoopdir/bin/hadoop jar $hadoopdir/contrib/streaming/hadoop-0.17.2-streaming.jar -input $inputs -output $outputs -mapper "./batch_retrieval -f $file.txt" -reducer cat -file  $dir/batch_retrieval -file $1  -jobconf mapred.reduce.tasks=5   -jobconf mapred.map.tasks=200  -jobconf mapred.min.split.size=300000000



