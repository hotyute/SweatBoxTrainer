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
TaxiPath* curPoint = nullptr;
ApproachPath* curApproach = nullptr;

void processRunways(Airport* airport);


int LoadSCT(std::string path) {
	std::string line;
	std::ifstream myfile(path);
	if (myfile.is_open())
	{
		std::string commentStart = ";";
		int line_number = 1;
		size_t process_count = 0;
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
				for (size_t i = 0; i < HEADERS.size(); ++i) {
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
							(int)atodd(args[5]), (int)atodd(args[6]), mode, args[8]);
						curAircraft->getDefaultValues().speed = atodd(args[4]);
						//curAircraft->getDefaultValues().turn_rate = atodd(args[4]);
						curAircraft->apt_icao = args[9];
						processed_lines++;
					}
					else if (processed_lines == 1)
					{
						std::vector<std::string> args = split(line, ":");
						std::string rules = args[0];
						int mode = rules[0] == 'I' ? 1 : rules[0] == 'V' ? 0 : 2;

						if (curAircraft)
						{
							FlightPlan& fp = curAircraft->getFlightPlan();
							fp.acType = args[1].length() > 9 ? args[1].substr(0, 8) : args[1];
							fp.departure = args[2].length() > 4 ? args[2].substr(0, 3) : args[2];
							fp.arrival = args[3].length() > 4 ? args[3].substr(0, 3) : args[3];
							fp.route = args[4].length() > 128 ? args[4].substr(0, 127) : args[4];
							fp.cruise = FormatAltitude(args[6].length() > 6 ? args[6].substr(0, 5) : args[6]);
							fp.alternate = args[7].length() > 4 ? args[7].substr(0, 3) : args[7];
							fp.remarks = args[8].length() > 128 ? args[8].substr(0, 127) : args[8];
							++fp.cycle;
						}
						processed_lines++;
					}
					else if (processed_lines == 2)
					{
						std::vector<std::string> args = split(line, ":");
						if (curAircraft)
						{
							curAircraft->getPerfValues().takeoff_accel = atodd(args[0]);
							curAircraft->getPerfValues().v1 = atodd(args[1]);
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
		std::string fieldElevStart = "field elevation=";
		std::string turnoffStart = "turnoff=";
		std::string displacedStart = "displaced threshold=";
		std::string apprhdgStart = "heading=";
		std::string gsStart = "glideslope=";
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
					size_t field_elev = line.find(fieldElevStart);


					if (line[0] == '[' && line.back() == ']') {
						if (curAirport) {
							headerId = -1;
							std::string type = line.substr(1, line.length() - 2);
							std::vector<std::string> header = split(type, " ");
							if (header.size() == 2)
							{
								curApproach = nullptr;
								curPoint = nullptr;
								std::string name = header[1];
								capitalize(name);
								if (header[0] == "PARKING")
								{
									curPoint = new Parking();
									curPoint->name = name;
								}
								else if (header[0] == "RUNWAY")
								{
									curPoint = new Runway();
									curPoint->name = name;
								}
								else if (header[0] == "TAXIWAY")
								{
									curPoint = new Taxiway();
									curPoint->name = name;
								}
								else if (header[0] == "ILS")
								{
									curApproach = new ILS();
									curApproach->name = name;
								}
								else if (header[0] == "LOC")
								{
									curApproach = new LOC();
									curApproach->name = name;
								}
							}
						}
					}
					else if (curAirport && curPoint)
					{
						if (curPoint->type == PATHTYPE::PARKING)
						{
							std::vector<std::string> args = split(line, " ");

							Point2* p = new Point2(atodd(args[1].c_str()), atodd(args[0].c_str()));
							p->index = pushBack(curPoint->points, p);

							curAirport->parking.push_back((Parking*)curPoint);
						}
						else if (curPoint->type == PATHTYPE::TAXIWAY)
						{
							std::vector<std::string> args = split(line, " ");

							Point2* p = new Point2(atodd(args[1].c_str()), atodd(args[0].c_str()));
							p->index = pushBack(curPoint->points, p);
							p->parent = curPoint;

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

								Point2* p = new Point2(atodd(args[1].c_str()), atodd(args[0].c_str()));
								p->index = pushBack(curPoint->points, p);
								p->parent = curPoint;
							}
							//curAirport->runways.push_back((Runway*)curPoint);
						}

						curAirport->all.emplace(curPoint->name, curPoint);
					}
					else if (curAirport && curApproach)
					{
						size_t apprhdg_tag = line.find(apprhdgStart);
						if (apprhdg_tag != std::string::npos)
						{
							int start = (apprhdg_tag + apprhdgStart.length());
							curApproach->h_degrees = atodd(line.substr(start));
						}
						else if (curApproach->type == APPRTYPE::ILS)
						{
							size_t gs_tag = line.find(gsStart);
							if (gs_tag != std::string::npos)
							{
								int start = (gs_tag + turnoffStart.length());
								curApproach->v_degrees = atodd(line.substr(start));
							}
							else
							{
								std::vector<std::string> args = split(line, " ");
								curApproach->point.x_ = atodd(args[1].c_str());
								curApproach->point.y_ = atodd(args[0].c_str());
							}
						}
						else if (curApproach->type == APPRTYPE::LOC)
						{
							std::vector<std::string> args = split(line, " ");
							curApproach->point.x_ = atodd(args[1].c_str());
							curApproach->point.y_ = atodd(args[0].c_str());
						}
					}

					if (icao_tag != std::string::npos)
					{
						int start = (icao_tag + icaoStart.length());
						std::string icao = line.substr(start);
						auto it = airports.find(icao);
						if (it != airports.end())
						{
							delete it->second;
							it = airports.erase(it);
						}
						curAirport = new Airport(icao);
						airports.emplace(icao, curAirport);
						curAirport->icao = icao;
					}
					else if (field_elev != std::string::npos)
					{
						int start = (field_elev + fieldElevStart.length());
						std::string fieldelev = line.substr(start);
						if (curAirport)
							curAirport->elevation = atodd(fieldelev);
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
		processRunways(curAirport);
		printf("[Loaded Airport Data: %s]\n", curAirport->icao.c_str());
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

void processRunways(Airport* airport)
{
	auto it = airport->all.begin();

	while (it != airport->all.end())
	{
		TaxiPath* path = it->second;
		if (path->type == PATHTYPE::RUNWAY)
		{
			std::vector<std::string> names = split(path->name, "/");

			if (names.size() > 1)
			{
				TaxiPath* other = new TaxiPath(*path);

				path->name = names[0];
				other->name = names[1];

				std::reverse(other->points.begin(), other->points.end());

				it = airport->all.erase(it);

				airport->runways.push_back((Runway*)other);
				airport->runways.push_back((Runway*)path);
				airport->all.insert(it, { names[0], path });
				airport->all.insert(it, { names[1], other });

				continue;
			}
		}

		++it;
	}
}
