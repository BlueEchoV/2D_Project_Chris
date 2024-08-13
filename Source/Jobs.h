#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

struct Job {
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
