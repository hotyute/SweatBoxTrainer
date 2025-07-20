#pragma once

#include <mutex>
#include "tools/thread_pool.h"
#include "airport.h"

extern std::unique_ptr<ThreadPool> g_threadPool;