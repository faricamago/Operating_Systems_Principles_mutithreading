

#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

// C++ barrier class (from lecture notes).
// -----------------------------------------------------------------------------
// You do not have to use it in your implementation. If you don't use it, you
// may delete it.
class simple_barrier {
  std::mutex m_;
  std::condition_variable cv_;
  int n_remaining_, count_;
  bool coin_;

  public:
  simple_barrier(int count = 1) { init(count); }
  void init(int count)
  {
    count_ = count;
    n_remaining_ = count_;
    coin_ = false;
  }
  bool wait()
  {
    if (count_ == 1) return true;
    std::unique_lock<std::mutex> lk(m_);
    if (n_remaining_ == 1) {
      coin_ = ! coin_;
      n_remaining_ = count_;
      cv_.notify_all();
      return true;
    }
    auto old_coin = coin_;
    n_remaining_--;
    cv_.wait(lk, [&]() { return old_coin != coin_; });
    return false;
  }
};
std::vector<int64_t> result;
std::atomic<bool> global_finished = false;
std::atomic<bool> prime_or_not = true;
std::atomic<int> track = 0;
std::atomic<int64_t> number_to_be_tested = 0;
std::vector<std::thread> threads;
simple_barrier barrier = simple_barrier();

// returns true if n is prime, otherwise returns false
// -----------------------------------------------------------------------------
// to get full credit for this assignment, you will need to adjust or even
// re-write the code in this function to make it multithreaded.
static bool is_prime(int64_t n,int i_for_thrd,int glb_nthrd)
{
  int64_t i = 5+6*i_for_thrd;
  int64_t max = sqrt((double)n);
  // handle trivial cases
  if (n < 2) return false;
  if (n <= 3) return true; // 2 and 3 are primes
  if (n % 2 == 0) return false; // handle multiples of 2
  if (n % 3 == 0) return false; // handle multiples of 3
  // try to divide n by every number 5 .. sqrt(n)  
  while (i <= max) {
    if (n % i == 0) return false;
    if (n % (i + 2) == 0) return false;
    i += 6*glb_nthrd;
  }
  // didn't find any divisors, so it must be a prime
  return true;
}

static void is_prime_2(int64_t n,int i_for_thrd,int glb_nthrd)
{
  int64_t i = 5 + 6*i_for_thrd;
  int64_t max = sqrt((double)number_to_be_tested);

    
  if (number_to_be_tested < 2) prime_or_not = false; 
  if (number_to_be_tested % 2 == 0) prime_or_not = false;
  if (number_to_be_tested % 3 == 0)prime_or_not = false;
  if(number_to_be_tested==2 || number_to_be_tested==3) prime_or_not = true; 
  

  while (i <= max ) {
    if(!prime_or_not) break;
    if (number_to_be_tested % i == 0) prime_or_not = false;
    if (number_to_be_tested % (i + 2) == 0) prime_or_not = false;
    i += 6*glb_nthrd; 
  }
}



// This function takes a list of numbers in nums[] and returns only numbers that
// are primes.
//
// The parameter n_threads indicates how many threads should be created to speed
// up the computation.
// -----------------------------------------------------------------------------
// You will most likely need to re-implement this function entirely.
// Note that the current implementation ignores n_threads. Your multithreaded
// implementation must use this parameter.

void thread_Function(int i_for_thread, int n_threads,std::vector<int64_t> nums){
  while(true){
    if(barrier.wait()== true){
      int index = track;
      number_to_be_tested = nums.at(index);
      track++;
      if ((int64_t)nums.size() == track){
        global_finished = true;
      }
    }

    barrier.wait();
    is_prime_2(number_to_be_tested,i_for_thread, n_threads);
    if(barrier.wait()== true){
      if(prime_or_not == true) {
         result.push_back(number_to_be_tested);
      }else{
        prime_or_not = true;
      }   
    }
    barrier.wait();
    if(global_finished==true) break;
  }
}
std::vector<int64_t>
detect_primes(const std::vector<int64_t> & nums, int n_threads)
{
  if(n_threads == 1)
  {
    for (auto num : nums)
    {
      if (is_prime(num,0,1)) result.push_back(num);
    }
    return result;
  }
  
  barrier.init(n_threads);
  for(int i = 0; i < n_threads; i++){
    threads.push_back(std::thread(thread_Function, i, n_threads, nums));
  }

  for (int i = 0; i < n_threads; i++) {
    if (threads[i].joinable()) threads[i].join();
  }

  return result;
}