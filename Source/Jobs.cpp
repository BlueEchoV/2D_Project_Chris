#include "Jobs.h"
#include "Utility.h"
#include "..\libs\tracy\tracy\Tracy.hpp"

#include <stdint.h>

std::mutex job_mutex;
std::vector<std::shared_ptr<Job>> jobs_list = {};

const int TOTAL_THREADS = 16;
std::counting_semaphore<200> semaphore(0);
std::atomic semaphore_count = 0;
std::vector<std::thread> threads;

int get_semaphore_count() {
	return semaphore_count;
}

// Returns a reference to the job for keeping track of them
// NOTE: This could return a value to increment the reference 
// counter
void add_job(Job_Type type, void* data) {
	Job new_job = {};
	new_job.type = type;
	new_job.data = data;
	job_mutex.lock();
	jobs_list.push_back(std::make_shared<Job>(new_job));
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
		std::shared_ptr<Job> job = {};
		if (!jobs_list.empty()) {
			job = jobs_list.back();
			// This should delete the shared pointer
			// because the reference counter will be zero
			jobs_list.pop_back();
			job_available = true;
		}
		job_mutex.unlock();

		if (job_available) {
			execute_job_type(job->type, job->data);
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