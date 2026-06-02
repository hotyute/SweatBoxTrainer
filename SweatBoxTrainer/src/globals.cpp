#include "SweatBoxTrainer/globals.h"

// Define the global AppContext instance
AppContext g_app;

void AppContext::initialize() {
    // --- Initialize Thread Pool and Scheduler ---
    threadPool = std::make_unique<ThreadPool>();
    TimedTask::startSchedulerThread(); // The scheduler is still a static system

    // --- Initialize Tasks ---
    simulationTask = std::make_unique<SimulationTask>(*threadPool);
    guiUpdateTask = std::make_unique<GuiUpdateTask>(*threadPool);
    socketPollingTask = std::make_unique<SocketPollingTask>(*threadPool);

    // --- Start Tasks ---
    simulationTask->start();
    guiUpdateTask->start();
    socketPollingTask->start();
}

void AppContext::shutdown() {
    // --- Stop tasks in a safe order ---
    if (socketPollingTask) socketPollingTask->stop();
    if (guiUpdateTask) guiUpdateTask->stop();
    if (simulationTask) simulationTask->stop();

    // --- Stop the scheduler before destroying the thread pool ---
    TimedTask::stopSchedulerThread();

    // --- Explicitly destroy the thread pool ---
    // The unique_ptr will handle this automatically when g_app is destroyed,
    // but being explicit here is clearer.
    threadPool.reset();
}