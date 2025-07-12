// globals.cpp
#include "globals.h"

// Define the global mutex
std::mutex g_acfMapMutex;
std::unique_ptr<ThreadPool> g_threadPool;