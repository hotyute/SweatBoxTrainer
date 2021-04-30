#include "tools.h"

std::vector<std::string>& split(const std::string& str, const std::string& delimiters, std::vector<std::string>& elems, int times) {
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	if (times != -1)
	{
		while (times > 0 && (std::string::npos != pos || std::string::npos != lastPos)) {
			// Found a token, add it to the vector.
			elems.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);

			--times;
		}
	}
	else
	{
		while (std::string::npos != pos || std::string::npos != lastPos) {
			// Found a token, add it to the vector.
			elems.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	}
	return elems;
}


std::vector<std::string> split(const std::string& s, const std::string& delim, int times) {
	std::vector<std::string> elems;
	return split(s, delim, elems, times);
}

std::vector<std::string> split(const std::string& s, const std::string& delim) {
	return split(s, delim, -1);
}

char* s2ca1(const std::string& s) {
	char* res = new char[s.size() + 1];
	strncpy_s(res, s.size() + 1, s.c_str(), s.size() + 1);
	return res;
}

int random(int start, int end)
{
	if (end < start) {
		int temp = start;
		start = end;
		end = temp;
	}
	return start + (rand() % end);
}

long long doubleToRawBits(double x) {
	long long bits;
	memcpy(&bits, &x, sizeof bits);
	return bits;
}


