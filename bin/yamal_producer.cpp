#define FM_COUNTER_ENABLE

#include <iostream>

#include <fmc++/counters.hpp>
#include <fmc/process.h>

#include <cstring>
#include <fcntl.h>
#include <fmc/signals.h>
#include <iostream>
#include <random>
#include <signal.h>
#include <thread>
#include <ytp/yamal.h>

using namespace std;
using namespace fmc::counter;

static atomic<bool> run = true;

static void sig_handler(int s) { run = false; }

const uint64_t sleep_ns = 10000000ll;
const int msg_count = 4;

int main(int argc, char **argv) {

  fmc_set_signal_handler(sig_handler);

  if (argc != 2 && argc != 3) {
    cerr << "Incorrect number of arguments" << endl;
    cout << "Usage:" << endl;
    cout << "yamal_producer <yamal_file> [cpuid]" << endl;
    return -1;
  }

  const char *out_yamal_name = argv[1];

  fmc_error_t *error;
  fmc_fd out_yamal_fd = fmc_fopen(out_yamal_name, fmc_fmode::READWRITE, &error);
  if (!fmc_fvalid(out_yamal_fd)) {
    cerr << "Unable to open file " << out_yamal_name << endl;
    return -1;
  }

  int cpuid = -1;
  if (argc >= 3) {
    string cpuidstr = argv[2];
    char *end = nullptr;
    auto val = strtol(cpuidstr.data(), &end, 10);
    if (end == cpuidstr.data() + cpuidstr.size()) {
      cpuid = val;
    }
  }

  auto *yamal = ytp_yamal_new(out_yamal_fd, &error);

  if (yamal == nullptr) {
    cerr << fmc_error_msg(error) << endl;
    return -1;
  }

  bool has_error = false;

  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<int64_t> distr_sleep(0ll, sleep_ns);
  uniform_int_distribution<int> distr_count(0, msg_count);

  rdtsc_record<fmc::counter::log_bucket> record_commit;
  rdtsc_record<fmc::counter::avg> record_commit_avg;
  rdtsc_record<fmc::counter::min> record_commit_min;
  rdtsc_record<fmc::counter::max> record_commit_max;
  uint64_t total_count = 0;

  cout << "yamal_producer" << endl;

  const int priority = 100;

  if (cpuid >= 0) {
    auto cur_thread = fmc_tid_cur(&error);
    if (error) {
      cerr << "Error getting current thread: " << fmc_error_msg(error) << endl;
    } else {
      fmc_set_affinity(cur_thread, cpuid, &error);
      if (error) {
        cerr << "Error set affinity: " << fmc_error_msg(error) << endl;
      } else {
        fmc_set_sched_fifo(cur_thread, priority, &error);
        if (error) {
          cerr << "Error setting fifo scheduler: " << fmc_error_msg(error)
               << endl;
        }
      }
    }
  }

  do {
    this_thread::sleep_for(chrono::nanoseconds(distr_sleep(gen)));

    int count = distr_count(gen);

    for (int i = 0; i < count; ++i) {
      FMC_SCOPED_SAMPLE(record_commit);
      FMC_SCOPED_SAMPLE(record_commit_avg);
      FMC_SCOPED_SAMPLE(record_commit_min);
      FMC_SCOPED_SAMPLE(record_commit_max);

      int64_t timestamp = fmc::counter::rdtsc()();
      if (auto *data = ytp_yamal_reserve(yamal, sizeof(int64_t), &error);
          !error) {
        memcpy(data, &timestamp, sizeof(timestamp));

        if (ytp_yamal_commit(yamal, data, &error); error) {
          cerr << "could not commit the transaction" << endl;
          has_error = 1;
          break;
        }
        ++total_count;
      } else {
        cerr << "Unable to reserve memory for message" << endl;
        has_error = 1;
        break;
      }
    }
    this_thread::yield();
  } while (run && !has_error);

  ytp_yamal_del(yamal, &error);

  cout << "  WRITE to yamal:" << endl;
  cout << "                   Count: " << total_count << endl;
  cout << "                     Min: " << record_commit_min.value() << " ns"
       << endl;
  cout << "                 Average: " << record_commit_avg.value() << " ns"
       << endl;
  cout << "         Percentile 50th: " << record_commit.percentile(50) << " ns"
       << endl;
  cout << "         Percentile 90th: " << record_commit.percentile(90) << " ns"
       << endl;
  cout << "         Percentile 99th: " << record_commit.percentile(99) << " ns"
       << endl;
  cout << "       Percentile 99.9th: " << record_commit.percentile(99.9)
       << " ns" << endl;
  cout << "      Percentile 99.99th: " << record_commit.percentile(99.99)
       << " ns" << endl;
  cout << "                     Max: " << record_commit_max.value() << " ns"
       << endl
       << endl;

  return has_error;
}
