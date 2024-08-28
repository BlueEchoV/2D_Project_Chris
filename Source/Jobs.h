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
	JT_Generate_Chunk_Perlin,
	JT_Generate_Chunk_Faces,
	JT_Total
};

struct Job {
	Job_Type type;
	void* data;
	bool finished_executing_all_steps;
};

void init_job_system(void(*execute_job_type)(Job_Type, void*));
int get_semaphore_count();
void terminate_all_threads();
void add_job(Job_Type type, void* data);
