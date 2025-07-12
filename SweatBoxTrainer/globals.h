#pragma once

#include <mutex>
#include "tools/thread_pool.h"

extern std::mutex g_acfMapMutex; // Protects AcfMap and userStorage1
extern std::unique_ptr<ThreadPool> g_threadPool;