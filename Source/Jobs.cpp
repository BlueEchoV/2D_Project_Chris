#include "Jobs.h"
#include "Utility.h"
#include "..\libs\tracy\tracy\Tracy.hpp"

#include <stdint.h>

std::mutex job_mutex;
std::vector<Job> jobs = {};
int current_threads_executing_a_job = 0;
const int TOTAL_THREADS = 10;
std::counting_semaphore<100> semaphore(0);
std::vector<std::thread> threads;

bool threads_finished_executing_jobs() {
	if (current_threads_executing_a_job <= 0) {
		return true;
	}
	return false;
}

void add_job(Job_Type type, void* data) {
	job_mutex.lock();
	Job new_job;
	new_job.type = type;
	new_job.data = data;
	jobs.push_back(new_job);
    semaphore.release();  
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

void thread_worker(void(*execute_job_type)(Job_Type, void*)) {
    while (true) {
		// ZoneScoped;
		if (should_terminate_threads) {
			break;
		}

		job_mutex.lock();
		if (jobs.empty()) {
			job_mutex.unlock();
			continue;
		}

		// This should never stall because release
		// is directly linked to adding a job.
		// If job is not empty, then there is 
		// definitely a value to be taken from the 
		// semaphore
		semaphore.acquire();  

		Job job = jobs.back();  
		jobs.pop_back();
		job_mutex.unlock();

		current_threads_executing_a_job++;
		execute_job_type(job.type, job.data);  
		current_threads_executing_a_job--;
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

#if 0
void execute_all_jobs() {
    for (int i = 0; i < jobs.size(); i++) {
        semaphore.release();  
    }

    for (std::thread& thread : threads) {
        if (thread.joinable()) {
			// Wait for all threads to complete
            thread.join();          }
    }
}

void Job_Increment_Value::execute_job() {
	// lambda for execution
	std::thread new_thread([this]() {
		a++;
		log("a = %i", a);
	});
	// Detach the thread if you don't need to wait for it to finish
	new_thread.detach();
}

std::atomic<bool> ready(false);
std::vector<Job*> jobs = {};
std::vector<std::thread> threads = {};

// void init_threads() {
// 	std::thread t1();
// 	std::thread t2;
// }

void execute_jobs() {
	for (Job* job : jobs) {
		// If a thread is available, give the job over to a thread
		// while (thread is aviailble)
		job->execute_job();
	}
}

void finish_executing_all_threads() {
	for (std::thread& thread : threads) {
		thread.join();
	}
	std::erase_if(jobs, [](Job* job) {
		if (!job->job_executed) {
			delete job;
			return true;
		}
		return false;
	});
}
#endif
