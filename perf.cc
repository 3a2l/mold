#include "mold.h"

#include <functional>
#include <iomanip>
#include <ios>

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

TimerRecord::TimerRecord(std::string name, TimerRecord *parent)
  : name(name), parent(parent) {

  start = now_nsec();
  int64_t user_now = 0, sys_now = 0;
  get_process_times(user_now, sys_now);
  user = user_now;
  sys = sys_now;

  if (parent)
    parent->children.push_back(this);
}

void TimerRecord::stop() {
  if (stopped)
    return;
  stopped = true;

  end = now_nsec();
  int64_t user_now = 0, sys_now = 0;
  get_process_times(user_now, sys_now);
  user = user_now - user;
  sys = sys_now - sys;
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
