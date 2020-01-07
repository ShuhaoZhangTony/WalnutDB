#!/bin/bash

cd ..
cmake .
make -j4

function benchmarkRun {
		#####native execution
		echo "==benchmark:$benchmark -a $algo -n $Threads=="
    ./hashing -a $algo -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads >> output.txt
}

# Configurable variables
# Generate a timestamp
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.csv
algo=""
RSIZE=1
SSIZE=1
RPATH=""
SPATH=""
RKEY=0
SKEY=0
RTS=0
STS=0
Threads=40
for algo in SHJ_JM_NP PMJ_JM_NP RPJ_JM_NP #SHJ_JBCR_NP PMJ_JBCR_NP RPJ_JBCR_NP SHJ_HS_NP PMJ_HS_NP RPJ_HS_NP
do
  for benchmark in "Stock" "Rovio" "DEBS" "YSB" "Google" "Amazon"
  do
    case "$benchmark" in
      # Stream
      "Stock")
        RSIZE=15010279
        SSIZE=15280728
        RPATH=/data1/xtra/datasets/stock/cj_key32_partitioned_preprocessed.csv
        SPATH=/data1/xtra/datasets/stock/sb_key32_partitioned_preprocessed.csv
        RKEY=0
        SKEY=0
        RTS=1
        STS=1
        benchmarkRun
    ;;
      "Rovio")
        RSIZE=999997
        SSIZE=999997
        RPATH=/data1/xtra/datasets/rovio/rovio_key32_partitioned.txt
        SPATH=/data1/xtra/datasets/rovio/rovio_key32_partitioned.txt
        RKEY=0
        SKEY=0
        RTS=3
        STS=3
        benchmarkRun
    ;;
      # Batch-Stream
      "YSB")
        RSIZE=1001
        SSIZE=749900
        RPATH=/data1/xtra/datasets/YSB/campaigns_key32_partitioned.txt
        SPATH=/data1/xtra/datasets/YSB/ad_events_key32_partitioned.txt
        RKEY=0
        SKEY=0
        RTS=0
        STS=1
        benchmarkRun
    ;;
      # Batch
      "DEBS")
        RSIZE=1003605
        SSIZE=2052169
        RPATH=/data1/xtra/datasets/DEBS/posts_key32_partitioned.csv
        SPATH=/data1/xtra/datasets/DEBS/comments_key32_partitioned.csv
        RKEY=0
        SKEY=0
        benchmarkRun
    ;;
        "Google")
        RSIZE=3747939
        SSIZE=11931801
        RPATH=/data1/xtra/datasets/google/users_key32_partitioned.csv
        SPATH=/data1/xtra/datasets/google/reviews_key32_partitioned.csv
        RKEY=1
        SKEY=1
        benchmarkRun
    ;;
        "Amazon")
        RSIZE=10
        SSIZE=10
        RPATH=/data1/xtra/datasets/amazon/amazon_question_partitioned.csv
        SPATH=/data1/xtra/datasets/amazon/amazon_answer_partitioned.csv
        RKEY=0
        SKEY=0
        benchmarkRun
    ;;
    esac
  done
done