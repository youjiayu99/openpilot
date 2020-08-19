#include <iostream>
#include <chrono>
#include <unistd.h>

#include <sys/syscall.h>
#include <sys/resource.h>


int main(void){

  int err;
  
  // SCHED_FIFO with highest priority
  long tid = syscall(SYS_gettid);
  struct sched_param sa;
  memset(&sa, 0, sizeof(sa));
  sa.sched_priority = 99;
  err = sched_setscheduler(tid, SCHED_FIFO, &sa);
  assert(err == 0);

  // core 3 is our shielded core
  cpu_set_t cpu;
  CPU_ZERO(&cpu);
  CPU_SET(3, &cpu);
  err = sched_setaffinity(tid, sizeof(cpu), &cpu);
  assert(err == 0);

  struct rusage ru;
  long switches_prev = 0;
  while (true){
    long sleep = 1000; // 1ms
    
    auto t_start = std::chrono::high_resolution_clock::now();
    usleep(sleep);
    auto t_end = std::chrono::high_resolution_clock::now();
    double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end-t_start).count();
    if (elapsed_time_ms > 5.0) {
      std::cout << "Sleep too long: " << elapsed_time_ms << std::endl;
    }
    err = getrusage(RUSAGE_SELF, &ru);
    if (switches_prev != ru.ru_nivcsw) {
      std::cout << "Involuntary context switch: " << switches_prev << " -> " << ru.ru_nivcsw  << std::endl;
      switches_prev = ru.ru_nivcsw;
    }
  }

  return 0;
}
