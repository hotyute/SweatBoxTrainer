#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>

int LoadSCT(std::string path);
void handleHeader(std::string& line);
void handleVORLine(std::string line);
void handleNDBLine(std::string line);
void handleAIRPORTLine(std::string line);
void handleFIXESLine(std::string line);
int LoadAPT(std::string path);

#endif