#include "Jobs.h"
#include "Utility.h"
#include "..\libs\tracy\tracy\Tracy.hpp"

#include <stdint.h>

std::atomic increment_value = 0;
void job_increment_number() {
	for (int i = 0; i < 1000; i++) {
		increment_value++;
	}
	log("Increment value = %i\n", (int)increment_value);
}

void job_print_stars() {
	for (int i = 0; i < 3; i++) {
		log("*****************\n");
	}
}

std::mutex job_mutex;
std::vector<Job_Type> jobs = {};
std::atomic current_threads_executing_a_job = 0;
const int TOTAL_THREADS = 10;
std::counting_semaphore<100> semaphore(0);
std::vector<std::thread> threads;
void add_job(Job_Type type) {
	job_mutex.lock();
	jobs.push_back(type);
    semaphore.release();  
	job_mutex.unlock();
}

void execute_job_type(Job_Type type) {
	switch(type) {
	case JT_Increment_Number: {
		job_increment_number();
		break;
	}
	case JT_Print_Stars: {
		job_print_stars();
		break;
	}
	default: {
		log("Error: Invalid job type");
		assert(false);
		break;
	}
	}
}

bool should_terminate_threads = false;
void terminate_all_threads() {
	should_terminate_threads = true;
}

void thread_worker() {
    while (true) {
		ZoneScoped;
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

		Job_Type job = jobs.back();  
		jobs.pop_back();
		job_mutex.unlock();

		current_threads_executing_a_job++;
		execute_job_type(job);  
		current_threads_executing_a_job--;
    }
}

void init_job_system() {
	threads.reserve(TOTAL_THREADS);
	for (int i = 0; i < TOTAL_THREADS; i++) {
		threads.emplace_back(thread_worker);
	}
}

void ensure_threads_finished() {
	while (current_threads_executing_a_job > 0) {
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
