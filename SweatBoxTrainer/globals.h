#pragma once

#include <mutex>
#include "tools/thread_pool.h"
#include "airport.h"

extern std::mutex g_acfMapMutex; // Protects AcfMap and userStorage1
extern std::unique_ptr<ThreadPool> g_threadPool;

extern std::unordered_map<std::string, std::unique_ptr<Airport>> airports;