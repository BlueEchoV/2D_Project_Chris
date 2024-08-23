#include "Jobs.h"
#include "Utility.h"
#include "..\libs\tracy\tracy\Tracy.hpp"

#include <stdint.h>

std::mutex job_mutex;
std::vector<Job> jobs_list = {};
std::mutex job_finished_mutex = {};
std::vector<Job> jobs_finished = {};

const int TOTAL_THREADS = 16;
std::counting_semaphore<200> semaphore(0);
std::atomic semaphore_count = 0;
std::vector<std::thread> threads;

int get_semaphore_count() {
	return semaphore_count;
}

// Returns a reference to the job for keeping track of them
void add_job(Job_Type type, void* data) {
	Job new_job = {};
	new_job.type = type;
	new_job.data = data;
	new_job.finished_executing_all_steps = false;
	job_mutex.lock();
	jobs_list.push_back(new_job);
	job_mutex.unlock();
    semaphore.release();  
	semaphore_count++;
}

bool should_terminate_threads = false;
void terminate_all_threads() {
	should_terminate_threads = true;
	for (int i = 0; i < TOTAL_THREADS; i++) {
		// Wakeup all the threads to check the 
		// should_terminate_threads variable
		semaphore.release();
		semaphore_count++;
	}
}

void thread_worker(void(*execute_job_type)(Job_Type, void*)) {
    while (true) {
		// ZoneScoped;
		if (should_terminate_threads) {
			break;
		}

		// This should never stall because release
		// is directly linked to adding a job.
		// If job is not empty, then there is 
		// definitely a value to be taken from the 
		// semaphore
		semaphore.acquire();

		if (should_terminate_threads) {
			break;
		}
		bool job_available = false;
		job_mutex.lock();
		Job job = {};
		if (!jobs_list.empty()) {
			job = jobs_list.back();
			jobs_list.pop_back();
			job_available = true;
		}
		job_mutex.unlock();

		if (job_available) {
			execute_job_type(job.type, job.data);
			job.finished_executing_all_steps = false;
			job_finished_mutex.lock();
			jobs_finished.push_back(job);
			job_finished_mutex.unlock();
		}

		semaphore_count--;
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