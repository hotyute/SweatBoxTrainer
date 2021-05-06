#ifndef AIRPORT_H
#define AIRPORT_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "point2d.h"

enum class PATHTYPE {NONE, PARKING, TAXIWAY, RUNWAY};

class TaxiPath {
public:
	virtual ~TaxiPath() {
		for (auto p : points)
		{
			delete p;
		}
	}
	PATHTYPE type = PATHTYPE::NONE;
	std::string name;
	std::vector<Point2*> points;
};

class Parking : public TaxiPath {
public:
	Parking() {
		type = PATHTYPE::PARKING;
	}
	virtual ~Parking() {}
};

class Taxiway : public TaxiPath {
public:
	Taxiway() {
		type = PATHTYPE::TAXIWAY;
	}
	virtual ~Taxiway() {}
};

class Runway : public TaxiPath {
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
public:
	Airport(std::string apt_icao) : icao(apt_icao) {}
	double elevation = 0;
	double pattern_elevation = 0;
	double vfr_init_altitude = 0;
	double ifr_init_altitude = 0;
	std::unordered_map<std::string, TaxiPath*> all;
	std::vector<Taxiway*> taxiway;
	std::vector<Runway*> runways;
	std::vector<Parking*> parking;
	std::string icao;
};

extern std::unordered_map<std::string, Airport*> airports;

#endif