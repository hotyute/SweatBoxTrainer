#ifndef AIRPORT_H
#define AIRPORT_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "point2d.h"

enum class PATHTYPE {NONE, PARKING, TAXIWAY, RUNWAY};

class DataPoint {
public:
	virtual ~DataPoint() {
		for (auto p : points)
		{
			delete p;
		}
	}
	PATHTYPE type = PATHTYPE::NONE;
	std::string name;
	std::vector<Point2*> points;
};

class Parking : public DataPoint {
public:
	Parking() {
		type = PATHTYPE::PARKING;
	}
	virtual ~Parking() {}
};

class Taxiway : public DataPoint {
public:
	Taxiway() {
		type = PATHTYPE::TAXIWAY;
	}
	virtual ~Taxiway() {}
};

class Runway : public DataPoint {
public:
	std::string turnoff;
	std::string displacement;
	Runway() {
		type = PATHTYPE::RUNWAY;
	}
	virtual ~Runway() {}
};

class Airport 
{
private:
	std::string ICAO;
public:
	Airport(std::string icao) : ICAO(icao) {}
	double elevation = 0;
	double pattern_elevation = 0;
	double vfr_init_altitude = 0;
	double ifr_init_altitude = 0;
	std::unordered_map<std::string, DataPoint*> all;
	std::vector<Taxiway*> taxiway;
	std::vector<Runway*> runways;
	std::vector<Parking*> parking;
};

extern std::unordered_map<std::string, Airport*> airports;

#endif