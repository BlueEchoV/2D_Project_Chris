#include "Jobs.h"
#include "Utility.h"

void Job_Increment_Value::execute_job() {
	std::thread new_thread([this]() {
		a++;
		log("a = %i", a);
	});
	// Detach the thread if you don't need to wait for it to finish
	new_thread.detach();
}

std::vector<Job*> jobs = {};
std::vector<std::thread> threads = {};

// void init_threads() {
// 	std::thread t1();
// 	std::thread t2;
// }

void finish_executing_all_threads() {
	for (std::thread& thread : threads) {
		thread.join();
	}
}
