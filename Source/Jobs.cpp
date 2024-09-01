#include "Jobs.h"
#include "Utility.h"
#include <queue>
#include "..\libs\tracy\tracy\Tracy.hpp"

#include <stdint.h>

std::mutex job_mutex;
std::queue<Job> job_list = {};

const int TOTAL_THREADS = 16;
std::counting_semaphore<TOTAL_THREADS> semaphore(0);
std::vector<std::thread> threads;

// Returns a reference to the job for keeping track of them
// NOTE: This could return a value to increment the reference 
// counter
void add_job(Job_Type type, void* data) {
	Job new_job = {};
	new_job.type = type;
	new_job.data = data;
	job_mutex.lock();
	job_list.push(new_job);
	job_mutex.unlock();
}

bool should_terminate_threads = false;
void terminate_all_threads() {
	should_terminate_threads = true;
	for (int i = 0; i < TOTAL_THREADS; i++) {
		// Wakeup all the threads to check the 
		// should_terminate_threads variable
		semaphore.release();
	}
}

std::atomic<int> current_threads_awakened = 0;
void wake_up_threads() {
	job_mutex.lock();
	if (!job_list.empty() && current_threads_awakened <= TOTAL_THREADS) {
		semaphore.release();
		current_threads_awakened++;
	}
	job_mutex.unlock();
}

void thread_worker(void(*execute_job_type)(Job_Type, void*)) {
    while (true) {
		// ZoneScoped;
		if (should_terminate_threads) {
			break;
		}

		semaphore.acquire();

		if (should_terminate_threads) {
			break;
		}

		bool job_available = false;
		Job job = {};

		job_mutex.lock();
		if (!job_list.empty()) {
			job = job_list.front();
			job_list.pop();
			job_available = true;
		}
		job_mutex.unlock();

		if (job_available) {
			execute_job_type(job.type, job.data);
		}
		current_threads_awakened--;
    }
}

void init_job_system(void(*execute_job_type)(Job_Type, void*)) {
	threads.reserve(TOTAL_THREADS);
	execute_job_type = execute_job_type;
	for (int i = 0; i < TOTAL_THREADS; i++) {
		// Direct Call: thread_worker(execute_job_type) evaluates the 
		// function immediately, producing a void result, which is not 
		// what std::thread expects.
		// Lambda: The lambda captures the function pointer and postpones 
		// the execution until the thread is actually running, which is 
		// what std::thread needs to correctly start the thread with the 
		// given work.
		threads.emplace_back([execute_job_type]() {
			thread_worker(execute_job_type);
		});
	}
}