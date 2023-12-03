#include "guidialogue.h"

#include <vector>
#include "SweatBoxTrainer.h"
#include "tools.h"

#include <string>
#include <chrono>
#include <iomanip>

std::vector<std::string> EstimateWrappedLines(const std::wstring& text, int maxWidth) {
	std::vector<std::string> wrappedLines;
	int currentLineWidth = 0;
	std::string currentLine;

	HDC hdc = GetDC(console_text);
	HFONT hFont = (HFONT)SendMessage(console_text, WM_GETFONT, 0, 0);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	for (const wchar_t& character : text) {
		// Get character width using the selected font
		SIZE size;
		GetTextExtentPoint32(hdc, &character, 1, &size);
		int charWidth = size.cx;

		if (character == L'\n') {
			// Handle explicit line breaks
			wrappedLines.push_back(currentLine);
			currentLine.clear();
			currentLineWidth = 0;
		}
		else {
			currentLineWidth += charWidth;

			if (currentLineWidth > maxWidth) {
				// Wrap to the next line
				wrappedLines.push_back(currentLine);
				currentLine.clear();
				currentLineWidth = charWidth;
			}
		}

		// Append the character to the current line
		currentLine.push_back(static_cast<char>(character));
	}

	// Add the last line if there is any content
	if (!currentLine.empty()) {
		wrappedLines.push_back(currentLine);
	}

	// Cleanup
	SelectObject(hdc, hOldFont);
	ReleaseDC(console_text, hdc);

	return wrappedLines;
}


int GetControlWidth(HWND control) {
	RECT clientRect;
	GetClientRect(control, &clientRect);
	return clientRect.right - clientRect.left;
}

std::vector<std::wstring> consoleHistory;

int CalculateMaxVisibleLines(HWND console_text) {
	RECT clientRect;
	GetClientRect(console_text, &clientRect);

	// Assuming you've already set the font using SendMessage(console_text, WM_SETFONT, ...)
	HDC hdc = GetDC(console_text);
	HFONT hFont = (HFONT)SendMessage(console_text, WM_GETFONT, 0, 0);
	HFONT hFontOld = (HFONT)SelectObject(hdc, hFont);

	// Calculate the height of a single line in the current font
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	int lineHeight = tm.tmHeight;

	// Calculate the maximum visible lines based on the control's height
	int maxVisibleLines = clientRect.bottom / lineHeight;

	// Cleanup
	SelectObject(hdc, hFontOld);
	ReleaseDC(console_text, hdc);

	return maxVisibleLines;
}

template <typename T>
const T& custom_max(const T& a, const T& b) {
	return (a < b) ? b : a;
}

template <typename T>
const T& custom_min(const T& a, const T& b) {
	return (a > b) ? b : a;
}


// Function to update the static control
// Function to update the static control
void UpdateConsole(HWND console_text) {
	// Combine the last N lines into a single string with line breaks
	std::wstring combinedText;
	const int MaxLines = CalculateMaxVisibleLines(console_text);

	int startIdx = custom_max(0, static_cast<int>(consoleHistory.size()) - MaxLines);
	std::vector<std::wstring> lines;
	int estimatedLines = 0;

	for (int i = (consoleHistory.size() - 1); i >= startIdx && estimatedLines < MaxLines; --i) {
		auto wrappedLines = EstimateWrappedLines(consoleHistory[i], GetControlWidth(console_text));
		for (auto lineIt = wrappedLines.rbegin(); lineIt != wrappedLines.rend(); ++lineIt) {
			auto& line = *lineIt;
			lines.insert(lines.begin(), s2ws(line));
			estimatedLines++;
		}
	}

	int remainingLines = MaxLines - estimatedLines;

	// Append new lines up to the remainingLines count
	for (int i = 0; i < remainingLines; ++i) {
		combinedText += L"\n"; // No explicit line break needed
	}

	for (int i = 0; i < lines.size(); ++i) {
		combinedText += lines[i] + L"\n";
	}

	// Set the combined text to the static control
	SetWindowText(console_text, combinedText.c_str());
}



// Function to append text to the console control and update the history
void AppendTextToConsole(const std::wstring& text) {
	// Get the current time
	auto now = std::chrono::system_clock::now();
	auto timePoint = std::chrono::system_clock::to_time_t(now);

	// Format the current time as HH:MM:SS
	std::tm tm;
	localtime_s(&tm, &timePoint);
	std::wstringstream timeStream;
	timeStream << std::put_time(&tm, L"[%H:%M:%S]");

	// Append the text with the current time to the history vector
	std::wstring messageWithTime = timeStream.str() + L" " + text;
	consoleHistory.push_back(messageWithTime);

	// Update the static control
	UpdateConsole(console_text);
}