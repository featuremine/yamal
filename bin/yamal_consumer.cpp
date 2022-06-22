#define FM_COUNTER_ENABLE

#include <fcntl.h>
#include <fmc++/counters.hpp>
#include <fmc/files.h>
#include <fmc/process.h>
#include <iostream>
#include <memory.h>
#include <signal.h>
#include <string_view>
#include <thread>

#include <fmc/signals.h>
#include <ytp/yamal.h>

using namespace std;
using namespace fmc::counter;

static atomic<bool> run = true;

static void sig_handler(int s) { run = false; }

int main(int argc, char **argv) {

  fmc_set_signal_handler(sig_handler);

  if (argc != 2 && argc != 3) {
    cerr << "Incorrect number of arguments" << endl;
    cout << "Usage:" << endl;
    cout << "yamal_consumer <file_name> [cpuid]" << endl;
    return -1;
  }

  const char *file_name = argv[1];

  fmc_error_t *error;
  fmc_fd fd = fmc_fopen(file_name, fmc_fmode::READWRITE, &error);
  if (!fmc_fvalid(fd)) {
    cerr << "Unable to open file " << file_name << endl;
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

  auto *yamal = ytp_yamal_new(fd, &error);

  if (error) {
    cerr << "Unable to initialize mmlist" << endl;
    return -1;
  }

  ytp_iterator_t it = ytp_yamal_end(yamal, &error);

  rdtsc_record<fmc::counter::log_bucket> record_read;
  rdtsc_record<fmc::counter::avg> record_read_avg;
  rdtsc_record<fmc::counter::min> record_read_min;
  rdtsc_record<fmc::counter::max> record_read_max;

  rdtsc_record<fmc::counter::log_bucket> record_recv;
  rdtsc_record<fmc::counter::avg> record_recv_avg;
  rdtsc_record<fmc::counter::min> record_recv_min;
  rdtsc_record<fmc::counter::max> record_recv_max;

  uint64_t total_count = 0;

  cout << "yamal_consumer" << endl;

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
    while (!ytp_yamal_term(it)) {
      int64_t now_timestamp;
      int64_t recv_timestamp;
      {
        FMC_SCOPED_SAMPLE(record_read);
        FMC_SCOPED_SAMPLE(record_read_avg);
        FMC_SCOPED_SAMPLE(record_read_min);
        FMC_SCOPED_SAMPLE(record_read_max);

        size_t size;
        const char *buf;
        ytp_yamal_read(yamal, it, &size, (const char **)&buf, &error);

        memcpy(&recv_timestamp, buf, sizeof(recv_timestamp));
      }

      now_timestamp = fmc::counter::rdtsc()();

      auto sample = now_timestamp - recv_timestamp;

      record_recv.sample(sample);
      record_recv_avg.sample(sample);
      record_recv_min.sample(sample);
      record_recv_max.sample(sample);

      it = ytp_yamal_next(yamal, it, &error);
      ++total_count;
    }
    this_thread::yield();
  } while (run);

  ytp_yamal_del(yamal, &error);

  cout << "  READ from yamal:" << endl;
  cout << "                   Count: " << total_count << endl;
  cout << "                     Min: " << record_read_min.value() << " ns"
       << endl;
  cout << "                 Average: " << record_read_avg.value() << " ns"
       << endl;
  cout << "         Percentile 50th: " << record_read.percentile(50) << " ns"
       << endl;
  cout << "         Percentile 90th: " << record_read.percentile(90) << " ns"
       << endl;
  cout << "         Percentile 99th: " << record_read.percentile(99) << " ns"
       << endl;
  cout << "       Percentile 99.9th: " << record_read.percentile(99.9) << " ns"
       << endl;
  cout << "      Percentile 99.99th: " << record_read.percentile(99.99) << " ns"
       << endl;
  cout << "                     Max: " << record_read_max.value() << " ns"
       << endl
       << endl;

  cout << "  RECEIVE from producer:" << endl;
  cout << "                   Count: " << total_count << endl;
  cout << "                     Min: " << record_recv_min.value() << " ns"
       << endl;
  cout << "                 Average: " << record_recv_avg.value() << " ns"
       << endl;
  cout << "         Percentile 50th: " << record_recv.percentile(50) << " ns"
       << endl;
  cout << "         Percentile 90th: " << record_recv.percentile(90) << " ns"
       << endl;
  cout << "         Percentile 99th: " << record_recv.percentile(99) << " ns"
       << endl;
  cout << "       Percentile 99.9th: " << record_recv.percentile(99.9) << " ns"
       << endl;
  cout << "      Percentile 99.99th: " << record_recv.percentile(99.99) << " ns"
       << endl;
  cout << "                     Max: " << record_recv_max.value() << " ns"
       << endl
       << endl;

  return 0;
}
