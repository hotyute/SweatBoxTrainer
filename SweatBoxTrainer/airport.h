#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory> // Include for std::unique_ptr

#include "point2d.h"

enum class PATHTYPE { NONE, PARKING, TAXIWAY, RUNWAY };

enum class APPRTYPE { NONE, ILS, LOC };

class TaxiPath {
public:
	// Destructor is no longer needed, smart pointers will handle cleanup.
	virtual ~TaxiPath() = default;

	PATHTYPE type = PATHTYPE::NONE;
	std::string name;
	std::vector<Point2*> points;
	Point2* getPrevPoint(Point2* to, Point2* next);
	Point2* getNextPoint(Point2* last, Point2* p2);
	void getPoints(Point2* p, Point2* p2, std::vector<Point2*>& points);
	Point2* angleTest(const Point2& orig, Point2& p, Point2& p2);
	Point2* getClosestPoint(double latitude, double longitude);
	Point2* getClosest(TaxiPath* next);
	Point2* intersect(Point2&);
	Point2* getStart();
	Point2* getEnd();
};

class ApproachPath {
public:
	virtual ~ApproachPath() = default;
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
};

class Taxiway : public TaxiPath {
public:
	Taxiway() {
		type = PATHTYPE::TAXIWAY;
	}
};

class Runway : public TaxiPath {
public:
	std::string turnoff;
	std::string displacement;
	Runway() {
		type = PATHTYPE::RUNWAY;
	}
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

	// This map stores non-owning pointers for quick lookups.
	std::unordered_map<std::string, TaxiPath*> all;

	// These vectors now store unique_ptrs and OWN the path objects.
	std::vector<std::unique_ptr<ApproachPath>> approaches;
	std::vector<std::unique_ptr<Taxiway>> taxiway;
	std::vector<std::unique_ptr<Runway>> runways;
	std::vector<std::unique_ptr<Parking>> parking;

	std::string icao;
};

// This map of non-owning pointers is fine, as the objects will be owned by something else (e.g. an AppContext class)
extern std::unordered_map<std::string, Airport*> airports;