#include "filereader.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "tools.h"
#include "usermanager.h"
#include "airport.h"

std::vector<std::string> HEADERS = { "[VOR]", "[NDB]", "[AIRPORT]", "[FIXES]" };
int headerId = -1;
Aircraft* curAircraft = nullptr;
Airport* curAirport = nullptr;
std::string curIcao = "";
DataPoint* curPoint = nullptr;


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
	std::string line;
	std::ifstream myfile(path);
	if (myfile.is_open())
	{
		std::string commentStart = ";";
		std::string icaoStart = "icao=";
		std::string turnoffStart = "turnoff=";
		std::string displacedStart = "displaced threshold=";
		int line_number = 1;
		int processed_lines = 0;
		while (getline(myfile, line)) {
			size_t foundComment = line.find(commentStart);
			if (foundComment != std::string::npos) {
				line = line.substr(0, foundComment);
			}
			try
			{
				bool whiteSpacesOnly = std::all_of(line.begin(), line.end(), isspace);
				if (!whiteSpacesOnly && !empty(line) && line.length() > 0)
				{
					size_t icao_tag = line.find(icaoStart);


					if (line[0] == '[' && line.back() == ']') {
						if (curAirport) {
							headerId = -1;
							std::string type = line.substr(1, line.length() - 2);
							std::vector<std::string> header = split(type, " ");
							if (header.size() == 2)
							{
								if (header[0] == "PARKING")
								{
									curPoint = new Parking();
								}
								else if (header[0] == "RUNWAY")
								{
									curPoint = new Runway();
								}
							}
						}
					}
					else if (curAirport && curPoint)
					{
						if (curPoint->type == PATHTYPE::PARKING)
						{
							std::vector<std::string> args = split(line, " ");
							curPoint->points.push_back(new Point2(atodd(args[0].c_str()), atodd(args[1].c_str())));
							curAirport->parking.push_back((Parking*)curPoint);
						}
						else if (curPoint->type == PATHTYPE::TAXIWAY)
						{
							std::vector<std::string> args = split(line, " ");
							curPoint->points.push_back(new Point2(atodd(args[0].c_str()), atodd(args[1].c_str())));
							curAirport->taxiway.push_back((Taxiway*)curPoint);
						}
						else if (curPoint->type == PATHTYPE::RUNWAY)
						{
							Runway& runway = *((Runway*)curPoint);
							size_t turnoff_tag = line.find(turnoffStart);
							size_t displaced_tag = line.find(displacedStart);
							if (turnoff_tag != std::string::npos)
							{
								int start = (turnoff_tag + turnoffStart.length());
								runway.turnoff = line.substr(start);
							}
							else if (displaced_tag != std::string::npos)
							{
								int start = (displaced_tag + displacedStart.length());
								runway.displacement = line.substr(start);
							}
							else
							{
								std::vector<std::string> args = split(line, " ");
								curPoint->points.push_back(new Point2(atodd(args[0].c_str()), atodd(args[1].c_str())));
							}
							curAirport->runways.push_back((Runway*)curPoint);
						}
						curAirport->all.push_back(curPoint);
					}

					if (icao_tag != std::string::npos)
					{
						int start = (icao_tag + icaoStart.length());
						std::string icao = line.substr(start);
						curAirport = new Airport(icao);
						airports.emplace(icao, curAirport);
					}
				}
			}
			catch (...)
			{
				std::stringstream box_message;
				box_message << "Error in aprt file at line: " << line_number;
				//MessageBoxA(hWnd, box_message.str().c_str(), "Notice",
				//	MB_OK | MB_ICONINFORMATION);
			}
			++line_number;
		}
	}
	return 0;
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


