#pragma once

#include <memory>
#include "tools/thread_pool.h"
#include "sim/simulation_context.h"
#include "calc_cycles.h"       // For SimulationTask
#include "SweatBoxTrainer.h"   // For GuiUpdateTask
#include "clinc2.h"            // For SocketPollingTask
#include "guidialogue.h"       // For ConsoleLogger

// Forward declarations
class SimulationTask;
class GuiUpdateTask;
class SocketPollingTask;

class AppContext {
public:
    // Manager Objects
    std::unique_ptr<ThreadPool> threadPool;

    // Task Objects
    std::unique_ptr<SimulationTask> simulationTask;
    std::unique_ptr<GuiUpdateTask> guiUpdateTask;
    std::unique_ptr<SocketPollingTask> socketPollingTask;

    // UI and Data Objects
    ConsoleLogger consoleLogger;

    // Singleton Accessor (for convenience)
    SimulationContext& sim() {
        return SimulationContext::instance();
    }

    // Initialization and Shutdown
    void initialize();
    void shutdown();
};

// Declare the single global instance of our application context.
// This will be the only "major" global variable.
extern AppContext g_app;