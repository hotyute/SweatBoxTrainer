#pragma once

#include <Windows.h>
#include <string>
#include "tools/timed_task.h"

static const int CLIENT_PORT = 6809;
extern bool done;
extern HWND hWnd;		// Holds Our Window Handle
extern HWND console_text;

void connect();

void create_controls(HWND hWnd);

class GuiUpdateTask : public TimedTask {
public:
    // Run at 10Hz (100ms interval)
    GuiUpdateTask(ThreadPool& pool)
        : TimedTask(pool, std::chrono::milliseconds(100), false) {}

protected:
    void execute() override;
};
