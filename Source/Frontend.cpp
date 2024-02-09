#include "Frontend.h"

void queueCommand(std::vector<Command>& commandQueue, Command& command) {
	commandQueue.push_back(command);
}