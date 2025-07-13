#define NOMINMAX
#include "windows.h"

#include "guidialogue.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm> // For std::min

// Define the global instance
ConsoleLogger g_consoleLogger;

void ConsoleLogger::Init(HWND hConsole) {
    m_hConsole = hConsole;
    m_hFont = (HFONT)SendMessage(m_hConsole, WM_GETFONT, 0, 0);
}

void ConsoleLogger::Log(const std::wstring& message) {
    auto now = std::chrono::system_clock::now();
    auto timePoint = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &timePoint);
    std::wstringstream timeStream;
    timeStream << std::put_time(&tm, L"[%H:%M:%S] ");

    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_consoleHistory.push_back(timeStream.str() + message);
}

void ConsoleLogger::FlushToConsole() {
    if (!m_hConsole) return;

    bool needsUpdate = false;
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (!m_consoleHistory.empty()) { // Check the main history now
            // Move queued messages into the main history
            for (const auto& msg : m_messageQueue) {
                m_consoleHistory.push_back(msg);
            }
            m_messageQueue.clear();
            needsUpdate = true;
        }
    }

    if (!needsUpdate) {
        return;
    }

    if (m_consoleHistory.size() > MAX_HISTORY_LINES) {
        m_consoleHistory.erase(m_consoleHistory.begin(), m_consoleHistory.begin() + (m_consoleHistory.size() - MAX_HISTORY_LINES));
    }

    int visibleLines = CalculateMaxVisibleLines();
    if (visibleLines <= 0) return;

    std::wstringstream textBuffer;
    int linesToShow = std::min((int)m_consoleHistory.size(), visibleLines);
    int paddingLines = visibleLines - linesToShow;

    // Add padding with newlines at the TOP
    for (int i = 0; i < paddingLines; ++i) {
        textBuffer << L"\r\n";
    }

    // Add the most recent lines, avoiding a trailing newline
    int historyStartIndex = (int)m_consoleHistory.size() - linesToShow;
    for (int i = 0; i < linesToShow; ++i) {
        textBuffer << m_consoleHistory[historyStartIndex + i];
        if (i < linesToShow - 1) {
            textBuffer << L"\r\n";
        }
    }

    // Set the entire text at once
    SetWindowText(m_hConsole, textBuffer.str().c_str());
}

int ConsoleLogger::CalculateMaxVisibleLines() {
    if (!m_hConsole || !m_hFont) return 0;

    RECT clientRect;
    GetClientRect(m_hConsole, &clientRect);

    HDC hdc = GetDC(m_hConsole);
    HFONT hOldFont = (HFONT)SelectObject(hdc, m_hFont);

    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);

    SelectObject(hdc, hOldFont);
    ReleaseDC(m_hConsole, hdc);

    int lineHeight = tm.tmHeight + tm.tmExternalLeading;
    if (lineHeight <= 0) return 0;

    return (clientRect.bottom - clientRect.top) / lineHeight;
}

void AppendTextToConsole(const std::wstring& text) {
    g_consoleLogger.Log(text);
}