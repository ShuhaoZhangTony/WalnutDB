//
// Created by Shuhao Zhang on 17/10/19.
//


#include "../helper/launcher.h"
#include "../utils/barrier.h"
#include "t_timer.h"  /* startTimer, stopTimer */
#include <sys/time.h>           /* gettimeofday */
#include <stdlib.h>             /* memalign */
#include <stdio.h>              /* printf */
#include <string.h>             /* memset */
#include <pthread.h>            /* pthread_* */
#include <sstream>
#include <zconf.h>
#include "t_timer.h"


using namespace std;

std::string GetCurrentWorkingDir(void) {
    char buff[FILENAME_MAX];
    getcwd(buff, FILENAME_MAX);
    std::string current_working_dir(buff);
    return current_working_dir;
}

/**
 * print progressive results.
 * @param vector
 */
void
dump_timing(std::vector<std::chrono::milliseconds> vector,
            std::vector<int64_t> vector_latency,
            std::vector<int32_t> global_record_gap,
            std::string arg_name, int exp_id, long lastTS) {

    //print progressive
    int n = vector.size() - 1;
    int check10 = ceil(n * 0.10);
    int check25 = ceil(n * 0.25);
    int check50 = ceil(n * 0.5);
    int check75 = ceil(n * 0.75);
    std::chrono::milliseconds start = vector.at(0);

    fprintf(stdout, "Time to obtain 10%%, 25%%, 50%%, 75%% of results (MSECS): \n");
    fprintf(stdout, "(%.2lu) \t (%.2lu) \t (%.2lu) \t (%.2lu)",
            vector.at(check10).count() + lastTS - start.count(),
            vector.at(check25).count() + lastTS - start.count(),
            vector.at(check50).count() + lastTS - start.count(),
            vector.at(check75).count() + lastTS - start.count());
    fprintf(stdout, "\n");
    fprintf(stdout, "\n");
    fflush(stdout);

    //dump timestmap.
    std::string name = arg_name + "_" + std::to_string(exp_id);
    string path = "/data1/xtra/results/timestamps/" + name.append(".txt");
    ofstream outputFile(path, std::ios::trunc);
    auto begin = vector.begin().operator*();
    vector.erase(vector.begin());
    for (auto &element : vector) {
        outputFile << (std::to_string(element.count() - begin.count() + lastTS) + "\n");
    }
    outputFile.close();

    //dump latency
    std::string name_latency = arg_name + "_" + std::to_string(exp_id);
    string path_latency = "/data1/xtra/results/latency/" + name_latency.append(".txt");
    ofstream outputFile_latency(path_latency, std::ios::trunc);
    for (auto &element : vector_latency) {
        outputFile_latency << (std::to_string(element + lastTS) + "\n");
    }
    outputFile_latency.close();

    //dump gap
    std::string name_gap = arg_name + "_" + std::to_string(exp_id);
    string path_gap = "/data1/xtra/results/gaps/" + name_gap.append(".txt");
    ofstream outputFile_gap(path_gap, std::ios::trunc);
    for (auto &element : global_record_gap) {
        outputFile_gap << (std::to_string(element) + "\n");
    }
    outputFile_gap.close();
}

/**
 *
 * @param result
 * @param timer
 * @param lastTS  in millseconds, lazy algorithms have to wait until very last tuple arrive before proceed.
 */
void dump_breakdown(int64_t result, T_TIMER *timer, long lastTS, _IO_FILE *pFile) {
#ifndef NO_TIMING
    if (result != 0) {

        double diff_usec = (((timer->end).tv_sec * 1000000L + (timer->end).tv_usec)
                            - ((timer->start).tv_sec * 1000000L + (timer->start).tv_usec)) + lastTS * 1000L;

        if (lastTS != 0) {//lazy join algorithms.
            timer->wait_timer = (lastTS * 1E-3) * (2.1 * 1E9);//MYC: 2.1GHz -> 2.1 * 1E9 Hz (2.1 * 1E9 cycles / second)
            timer->overall_timer += timer->wait_timer;
        } else {//eager join algorithms.
            timer->partition_timer -= timer->wait_timer;//exclude waiting time during tuple shuffling.
            timer->partition_timer -= timer->join_partitiontimer;//exclude join time during tuple shuffling.
        }

        timer->join_timer += timer->join_mergetimer;

        double cyclestuple = (timer->overall_timer) / result;

        //for system to read.
        //only take one thread to dump?
        //WAIT, PART, BUILD, SORT, MERGE, JOIN, OTHERS
        fprintf(pFile, "%lu\n%lu\n%lu\n%lu\n%lu\n%lu\n%lu\n",
                timer->wait_timer,
                timer->partition_timer,
                timer->buildtimer,
                timer->sorttimer,
                timer->mergetimer,
                timer->join_timer,
                timer->overall_timer -
                (timer->wait_timer + timer->partition_timer + timer->buildtimer + timer->sorttimer + timer->mergetimer +
                 timer->join_timer)
        );
        fprintf(pFile, "===\n");

        //for user to read.
        fprintf(pFile, "[Info] RUNTIME TOTAL, WAIT, PART, BUILD, SORT, MERGE, JOIN (cycles): \n");
        fprintf(pFile, "%llu \t %llu (%.2f%%) \t %llu (%.2f%%) \t %llu (%.2f%%)  "
                       "\t %llu (%.2f%%)  \t %llu (%.2f%%) \t %llu (%.2f%%)",
                timer->overall_timer,
                timer->wait_timer, (timer->wait_timer * 100 / (double) timer->overall_timer),
                timer->partition_timer, (timer->partition_timer * 100 / (double) timer->overall_timer),
                timer->buildtimer, (timer->buildtimer * 100 / (double) timer->overall_timer),
                timer->sorttimer, (timer->sorttimer * 100 / (double) timer->overall_timer),
                timer->mergetimer, (timer->mergetimer * 100 / (double) timer->overall_timer),
                timer->join_timer, (timer->join_timer * 100 / (double) timer->overall_timer)
        );
        fprintf(pFile, "\n");
        fprintf(pFile, "TOTAL-TIME-USECS, NUM-TUPLES, CYCLES-PER-TUPLE: \n");
        fprintf(pFile, "%.4lf \t %ld \t %.4lf", diff_usec, result, cyclestuple);
        fprintf(pFile, "\n");
        fprintf(pFile, "\n");
    } else {
        fprintf(stdout, "[Warning] This thread does not matches any tuple.\n\n");
    }
    fflush(pFile);
#endif
}

milliseconds actual_start_timestamp;
std::vector<std::chrono::milliseconds> global_record;
std::vector<int64_t> global_record_latency;
std::vector<int32_t> global_record_gap;

void merge(T_TIMER *timer, relation_t *relR, relation_t *relS, std::chrono::steady_clock::time_point *startTS) {
#ifndef NO_TIMING
    //For progressiveness measurement
    actual_start_timestamp = duration_cast<milliseconds>(startTS->time_since_epoch());
    for (auto i = 0; i < timer->recordR.size(); i++) {
        global_record.push_back(timer->recordR.at(i));
    }
    for (auto i = 0; i < timer->recordS.size(); i++) {
        global_record.push_back(timer->recordS.at(i));
    }
    //For latency and disorder measurement
    int64_t latency = -1;
    int32_t gap = 0;
    for (auto i = 0; i < timer->recordRID.size(); i++) {
        latency =
                timer->recordR.at(i).count() - startTS->count()
                - relR->payload->ts[timer->recordRID.at(i)].count();//latency of one tuple.
        global_record_latency.push_back(latency);
        gap = timer->recordRID.at(i) - i;//if it's sequentially processed, gap should be zero.
        global_record_gap.push_back(gap);
    }
    for (auto i = 0; i < timer->recordSID.size(); i++) {
        latency =
                timer->recordS.at(i).count() - startTS->count() -
                relS->payload->ts[timer->recordSID.at(i)].count();//latency of one tuple.
        global_record_latency.push_back(latency);
        gap = timer->recordSID.at(i) - i;//if it's sequentially processed, gap should be zero.
        global_record_gap.push_back(gap);
    }
#endif
}

/**
 * TODO: check why the second sort causes many memory errors.
 * "Conditional jump or move depends on uninitialised value(s)"
 * @param algo_name
 */
void sortRecords(std::string algo_name, int exp_id, long lastTS) {

    //sort the global record to get to know the actual time when each match success.
    global_record.push_back(actual_start_timestamp);
    sort(global_record.begin(), global_record.end());
    sort(global_record_latency.begin(), global_record_latency.end());
    sort(global_record_gap.begin(), global_record_gap.end());
    /* now print the progressive results: */
    dump_timing(global_record, global_record_latency, global_record_gap, algo_name, exp_id, lastTS);

}

template<typename Clock>
typename Clock::time_point now() {
    return chrono::steady_clock::now();
}

