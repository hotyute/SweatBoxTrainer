#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include "SweatBoxTrainer/point2d.h"

enum class PATHTYPE { NONE, PARKING, TAXIWAY, RUNWAY };
enum class APPRTYPE { NONE, ILS, LOC };

class TaxiPath {
public:
    virtual ~TaxiPath() = default;
    PATHTYPE type = PATHTYPE::NONE;
    std::string name;
    std::vector<std::unique_ptr<Point2>> nodes;

    Point2* getPrevPoint(Point2* to, Point2* next);
    Point2* getNextPoint(Point2* last, Point2* p2);
    void getPoints(Point2* p, Point2* p2, std::vector<Point2*>& out);
    Point2* angleTest(const Point2& orig, Point2& p, Point2& p2);
    Point2* getClosestPoint(double latitude, double longitude);
    Point2* getClosest(TaxiPath* next);
    Point2* intersect(Point2& p);
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

class Parking : public TaxiPath { public: Parking() { type = PATHTYPE::PARKING; } };
class Taxiway : public TaxiPath { public: Taxiway() { type = PATHTYPE::TAXIWAY; } };

class Runway : public TaxiPath {
public:
    std::string turnoff;
    std::string displacement;
    Runway() { type = PATHTYPE::RUNWAY; }
};

class ILS : public ApproachPath { public: ILS() { type = APPRTYPE::ILS; } };
class LOC : public ApproachPath { public: LOC() { type = APPRTYPE::LOC; } };

class Airport {
public:
    explicit Airport(std::string icao_) : icao(std::move(icao_)) {}
    double elevation = 0;
    double pattern_elevation = 0;
    double vfr_init_altitude = 0;
    double ifr_init_altitude = 0;

    std::unordered_map<std::string, TaxiPath*> all;
    std::vector<std::unique_ptr<ApproachPath>> approaches;
    std::vector<std::unique_ptr<Taxiway>> taxiway;
    std::vector<std::unique_ptr<Runway>> runways;
    std::vector<std::unique_ptr<Parking>> parking;
    std::string icao;
};