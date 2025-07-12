#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <Windows.h>

class ConsoleLogger {
public:
    void Init(HWND hConsole);
    void Log(const std::wstring& message);
    void FlushToConsole();

private:
    int CalculateMaxVisibleLines();

    HWND m_hConsole = nullptr;
    HFONT m_hFont = nullptr;

    // We only need one history vector, which is the true source of data
    std::vector<std::wstring> m_consoleHistory;
    std::vector<std::wstring> m_messageQueue;
    std::mutex m_queueMutex;

    static const int MAX_HISTORY_LINES = 500;
};

extern ConsoleLogger g_consoleLogger;
void AppendTextToConsole(const std::wstring& text);