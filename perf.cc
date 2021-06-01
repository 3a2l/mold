#include "mold.h"

#include <functional>
#include <iomanip>
#include <ios>

#ifdef WIN32
#else
#include <sys/resource.h>
#include <sys/time.h>
#endif

i64 Counter::get_value() {
  return values.combine(std::plus());
}

void Counter::print() {
  sort(instances, [](Counter *a, Counter *b) {
    return a->get_value() > b->get_value();
  });

  for (Counter *c : instances)
    std::cout << std::setw(20) << std::right << c->name
              << "=" << c->get_value() << "\n";
}

static i64 now_nsec() {
#ifdef WIN32
    // TODO
    return -1;
#else
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (i64)t.tv_sec * 1000000000 + t.tv_nsec;
#endif
}

#ifndef WIN32
static i64 to_nsec(struct timeval t) {
  return (i64)t.tv_sec * 1000000000 + t.tv_usec * 1000;
}
#endif

TimerRecord::TimerRecord(std::string name, TimerRecord *parent)
  : name(name), parent(parent) {
#ifdef WIN32
    // TODO
#else
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  start = now_nsec();
  user = to_nsec(usage.ru_utime);
  sys = to_nsec(usage.ru_stime);

  if (parent)
    parent->children.push_back(this);
#endif
}

void TimerRecord::stop() {
#ifdef WIN32
    // TODO
#else
  if (stopped)
    return;
  stopped = true;

  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  end = now_nsec();
  user = to_nsec(usage.ru_utime) - user;
  sys = to_nsec(usage.ru_stime) - sys;
#endif
}

template <typename E>
Timer<E>::Timer(Context<E> &ctx, std::string name, Timer *parent) {
  record = new TimerRecord(name, parent ? parent->record : nullptr);
  ctx.timer_records.push_back(std::unique_ptr<TimerRecord>(record));
}

template <typename E>
Timer<E>::~Timer() {
  record->stop();
}

template <typename E>
void Timer<E>::stop() {
  record->stop();
}

static void print_rec(TimerRecord &rec, i64 indent) {
  printf(" % 8.3f % 8.3f % 8.3f  %s%s\n",
         ((double)rec.user / 1000000000),
         ((double)rec.sys / 1000000000),
         (((double)rec.end - rec.start) / 1000000000),
         std::string(indent * 2, ' ').c_str(),
         rec.name.c_str());

  sort(rec.children, [](TimerRecord *a, TimerRecord *b) {
    return a->start < b->start;
  });

  for (TimerRecord *child : rec.children)
    print_rec(*child, indent + 1);
}

template <typename E>
void Timer<E>::print(Context<E> &ctx) {
  tbb::concurrent_vector<std::unique_ptr<TimerRecord>> &records =
    ctx.timer_records;

  for (i64 i = records.size() - 1; i >= 0; i--)
    records[i]->stop();

  for (i64 i = 0; i < records.size(); i++) {
    TimerRecord &inner = *records[i];
    if (inner.parent)
      continue;

    for (i64 j = i - 1; j >= 0; j--) {
      TimerRecord &outer = *records[j];
      if (outer.start <= inner.start && inner.end <= outer.end) {
        inner.parent = &outer;
        outer.children.push_back(&inner);
        break;
      }
    }
  }

  std::cout << "     User   System     Real  Name\n";

  for (std::unique_ptr<TimerRecord> &rec : records)
    if (!rec->parent)
      print_rec(*rec, 0);

  std::cout << std::flush;
}

template class Timer<X86_64>;
template class Timer<I386>;
