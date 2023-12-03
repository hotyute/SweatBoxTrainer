#pragma once

#include <iostream>

int LoadSCT(std::string path);
int LoadAGC(std::string path);
int LoadAPT(std::string path);

void handleHeader(std::string& line);
void handleVORLine(std::string line);
void handleNDBLine(std::string line);
void handleAIRPORTLine(std::string line);
void handleFIXESLine(std::string line);

extern std::string LAST_AGC_PATH, LAST_APRT_DIR, LAST_SCT_PATH;