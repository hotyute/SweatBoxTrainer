#ifndef AIRPORT_H
#define AIRPORT_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "point2d.h"

enum class PATHTYPE {NONE, PARKING, TAXIWAY, RUNWAY};

enum class APPRTYPE { NONE, ILS, LOC };

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
	Point2* getNextPoint(Point2* last, Point2* p2);
	Point2* angleTest(Point2* orig, Point2* p, Point2* p2);
	Point2* getClosestPoint(double latitude, double longitude);
	Point2* getClosest(TaxiPath* next);
};

class ApproachPath {
public:
	APPRTYPE type = APPRTYPE::NONE;
	std::string name;
	Point2 point;
	double h_degrees = -1;
	double v_degrees = -1;
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

class ILS : public ApproachPath {
public:
	ILS() { type = APPRTYPE::ILS; }
};

class LOC : public ApproachPath {
public:
	LOC() { type = APPRTYPE::LOC; }
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
	std::vector<ApproachPath*> approaches;
	std::vector<Taxiway*> taxiway;
	std::vector<Runway*> runways;
	std::vector<Parking*> parking;
	std::string icao;
};

extern std::unordered_map<std::string, Airport*> airports;

#endif