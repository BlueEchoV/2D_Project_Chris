#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <semaphore>
#include <assert.h>

enum Job_Type {
	JT_Increment_Number,
	JT_Print_Stars,
	JT_Total
};

void init_job_system();
void terminate_all_threads();
void ensure_threads_finished();
void add_job(Job_Type type);

// In order, what to do:
// 1) Add a job to a job list
// 2) Have a list of threads that are created to grab the jobs and execute them
// 3) Wait until all threads are finised to progress

#if 0
struct Job {
	bool job_executed = false;
	virtual void execute_job() = 0;
};

struct Job_Increment_Value : Job {
	int a = 0;
	void execute_job() override;
};

extern std::vector<Job*> jobs;
extern std::vector<std::thread> threads;
// void init_threads();
void finish_executing_all_threads();
#endif
