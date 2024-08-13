#include "Jobs.h"
#include "Utility.h"

#include <stdint.h>

static int increment_value = 0;
void job_increment_number() {
	increment_value++;
	log("Increment value = %i", increment_value);
}

std::mutex job_mutex;
std::vector<Job_Type> jobs = {};
void add_job(Job_Type type) {
	std::lock_guard<std::mutex> lock(job_mutex);
	jobs.push_back(type);
}

void execute_job_type(Job_Type type) {
	switch(type) {
	case JT_Increment_Number: {
		job_increment_number();
		break;
	}
	default: {
		log("Error: Invalid job type");
		assert(false);
		break;
	}
	}
}

// Max 10 with a count of 0
std::counting_semaphore<100> semaphore(0);

const int TOTAL_THREADS = 10;
std::vector<std::thread> threads;

void thread_worker() {
    while (true) {
		// Lock the jobs vector to safely access it
		// The mutex is unlocked when it goes out of scope
		std::lock_guard<std::mutex> lock(job_mutex);

		if (jobs.empty()) {
			break;
		}

		// Wait for a job to be available
		// NOTE: Acquire will not allow the below code to 
		// execute if the increment value is 0
        semaphore.acquire();  

		// Get the last job
		Job_Type job = jobs.back();  
		// Remove it from the list
		jobs.pop_back();
		// Execute the job
		execute_job_type(job);  
    }
}

void init_job_system() {
	threads.reserve(TOTAL_THREADS);
	for (int i = 0; i < TOTAL_THREADS; i++) {
		// NOTE: This Creates a new thread and immediately starts executing 
		// the thread_worker function in that thread. 
		threads.emplace_back(thread_worker);
	}
}

void execute_all_jobs() {
    for (int i = 0; i < jobs.size(); i++) {
		// Signal the semaphore to wake up a thread
        semaphore.release();  
    }

    for (std::thread& thread : threads) {
        if (thread.joinable()) {
			// Wait for all threads to complete
            thread.join();          }
    }
}

#if 0
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
