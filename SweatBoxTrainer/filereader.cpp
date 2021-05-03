#include "filereader.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "tools.h"
#include "usermanager.h"

std::vector<std::string> HEADERS = { "[VOR]", "[NDB]", "[AIRPORT]", "[FIXES]" };
int headerId = -1;
Aircraft* curAircraft = nullptr;


int LoadSCT(std::string path) {
	std::string line;
	std::ifstream myfile(path);
	if (myfile.is_open())
	{
		std::string commentStart = ";";
		int line_number = 1;
		int process_count = 0;
		while (getline(myfile, line)) {
			size_t foundComment = line.find(commentStart);
			if (foundComment != std::string::npos) {
				line = line.substr(0, foundComment);
			}

			if (line[0] == '[' && line.back() == ']') {
				headerId = -1;
				if (process_count >= HEADERS.size())
				{
					break;
				}
			}

			bool first_line = false;
			try {
				for (int i = 0; i < HEADERS.size(); ++i) {
					std::string header = HEADERS[i];
					size_t found = line.find(header);
					if (found != std::string::npos)
					{
						first_line = true;
						headerId = i;
						process_count++;
						break;
					}
				}
				bool whiteSpacesOnly = std::all_of(line.begin(), line.end(), isspace);
				if (!whiteSpacesOnly && !first_line && !empty(line) && (line.length() > 0) && headerId != -1)
				{
					handleHeader(line);
				}
			}
			catch (...)
			{
				std::stringstream box_message;
				box_message << "Error in sct file at line: " << line_number;
				//MessageBoxA(hWnd, box_message.str().c_str(), "Notice",
				//	MB_OK | MB_ICONINFORMATION);
			}
			++line_number;
		}
		myfile.close();
		return 1;
	}
	else
	{
		std::cout << "Unable to open file";
		return 0;
	}
}

void handleHeader(std::string& line) {
	switch (headerId) {
		case 0://VOR
		{
			handleVORLine(line);
			break;
		}
		case 1://NDB
		{
			handleNDBLine(line);
			break;
		}
		case 2://AIRPORT
		{
			handleAIRPORTLine(line);
			break;
		}
		case 3://FIXES
		{
			handleFIXESLine(line);
			break;
		}
	}
}

void handleVORLine(std::string line) {
	std::vector<std::string> args = split(line, " ");
}

void handleNDBLine(std::string line) {
	std::vector<std::string> args = split(line, " ");
}

void handleAIRPORTLine(std::string line) {
	std::vector<std::string> args = split(line, " ");
}

void handleFIXESLine(std::string line) {
	std::vector<std::string> args = split(line, " ");
}

int LoadAGC(std::string path) {
	std::string line;
	std::ifstream myfile(path);
	if (myfile.is_open())
	{
		std::string commentStart = ";";
		int line_number = 1;
		int processed_lines = 0;
		while (getline(myfile, line)) {
			size_t foundComment = line.find(commentStart);
			if (foundComment != std::string::npos) {
				line = line.substr(0, foundComment);
			}

			try {
				bool whiteSpacesOnly = std::all_of(line.begin(), line.end(), isspace);
				if (!whiteSpacesOnly && !empty(line) && line.length() > 0) 
				{
					if (processed_lines == 0)
					{
						std::vector<std::string> args = split(line, ":");
						std::string squawk_mode = args[7];
						int mode = squawk_mode[0] == 'C' ? 1 : squawk_mode[0] == 'I' ? 2 : 0;
						curAircraft = createAircraft(args[0], atodd(args[1]), atodd(args[2]), atodd(args[3]), atodd(args[4]),
							atodd(args[5]), atodd(args[6]), mode, args[8]);
						processed_lines++;
					}
					else if (processed_lines == 1)
					{
						std::vector<std::string> args = split(line, ":");
						std::string rules = args[0];
						int mode = rules[0] == 'I' ? 1 : rules[0] == 'V' ? 0 : 2;

						if (curAircraft)
						{
							FlightPlan& fp = *curAircraft->getFlightPlan();
							fp.acType = args[1];
							fp.departure = args[2];
							fp.route = args[3];
							fp.remarks = args[7];
							++fp.cycle;
						}
						processed_lines = 0;
					}
				}
			}
			catch (...)
			{
				std::stringstream box_message;
				box_message << "Error in agc file at line: " << line_number;
				//MessageBoxA(hWnd, box_message.str().c_str(), "Notice",
				//	MB_OK | MB_ICONINFORMATION);
			}
			++line_number;
		}
		myfile.close();
		return 1;
	}
	else
	{
		std::cout << "Unable to open file";
		return 0;
	}
}

int LoadAPT(std::string path)
{
	return 0;
}
