#ifndef __TOOLS_H
#define __TOOLS_H

#include <iostream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

std::vector<std::string> split(const std::string&, const std::string&, int times);

std::vector<std::string> split(const std::string& s, const std::string& delim);

int random(int start, int end);

#endif
