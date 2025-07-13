// globals.cpp
#include "globals.h"

// Define the global mutex
std::mutex g_acfMapMutex;
std::unique_ptr<ThreadPool> g_threadPool;

std::unordered_map<std::string, std::unique_ptr<Airport>> airports;