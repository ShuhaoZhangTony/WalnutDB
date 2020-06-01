<p align="left">
  <a href="https://github.com/ShuhaoZhangTony/AllianceDB/actions">
    <img alt="GitHub Actions status" src="https://github.com/ShuhaoZhangTony/AllianceDB/workflows/Main%20workflow/badge.svg"></a>
</p>

# What?
AllianceDB is a library for a list of stream operations optimized on modern multicore processors.

# How to Reproduce Our Experimental Results?

All of our experiments can be automatically reproduced by calling a few pre-prepared scripts.

### Step 1
git clone https://github.com/ShuhaoZhangTony/AllianceDB.git

### Step 2
open hashing/scripts/benchmark.sh
   At Line 4, set L3 cache size according to your machine specification;
   run the benchmark.sh to conduct experiments.
   
Remember to adjust the following lines in the scripts according to your needs.
  1. Line 299 to 427. 
     conducts most experiments for four datasets and micro-datasets.
     At Line 296, set profile_breakdown=1 if we want to measure time breakdown.
  2. Line 432 to 477.
     conducts the scalability test for four datasets. In our paper, we only show the results of YSB as an example.
  3. Line 495 to 562.
      conducts the cache miss profiling study using YSB as an example. 
      For this test, you must run the script with sudo, i.e., "sudo ./benchmark.sh".
      After the test, you have to manually fill in the results to "profile_ysb_partition.py" and "profile_ysb_probe.py" corresponding to the test IDs.
  4. Line 566 to 673.
     conducts experiments for algorithm parameters tuning.
### Step 3
Repeat the experiments for sorting/scripts/benchmark.sh

### Step 4
call ./draw.sh to generate figures.

  
# What's Next?
We start with stream join, which is argubly the most fundamental and difficult to implement operations.
In the future, we will add other relational operations including stream aggregation, filtering, group-by and so on.
  


