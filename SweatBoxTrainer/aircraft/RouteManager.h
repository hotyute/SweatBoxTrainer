#pragma once

#include <string>
#include <vector>
#include "../point2d.h" // Note the path
#include "../airport.h"
#include "../constants.h"

// Forward declarations
class Airport;
class Aircraft;
struct AircraftState;
struct AssignedValues;
struct DefaultValues;
struct PerfValues;
class TaxiPath;
class Runway;

class RouteManager {
public:
    // Navigation Data
    std::vector<std::string> ground_route;
    std::vector<Point2*> ground_points, holds;
    Point2* init_crse_p = nullptr;
    Point2* ground_prev = nullptr, * ground_cur = nullptr, * ground_next = nullptr, * ground_next_next = nullptr;
    Point2* air_prev = nullptr, * air_cur = nullptr;
    Runway* runway_ctx = nullptr;
    std::string apt_icao = "";

    // Holding Data
    Aircraft* HoldingFor = nullptr;
    TaxiPath* HoldingAt = nullptr, * HoldingDepart = nullptr;

    // Flags
    bool point_skip = false, locked_rate = false, queue_takeoff = false, lineup = false;

    // Public Methods
    void prepareRoute(Airport* airport, const AircraftState& state);
    bool pollRoute();
    void resetPath(AssignedValues& assigned, DefaultValues& defaults);
    void resetHolding();
    void resetContext();
    void HoldAt(Airport* airport, std::string name);
    
    // This is the main navigation logic function
    void updateNavigation(ACF_STATE& aircraft_state, AircraftState& state, AssignedValues& assigned, DefaultValues& defaults, const PerfValues& perf, Airport* airport);

    bool onGround(ACF_STATE acf_state) const;
    bool isTaxing() const;
    bool isHoldingForTakeoff() const;
    bool OnTrack(const AircraftState& state);

private:
    double calculateGS(const AircraftState& state, double station_lat, double station_lon, double thresh_ft, double capture_ft);
    double calculateLoc(const AircraftState& state, const AssignedValues& assigned, double dest_lat, double dest_lon, double loc_brg, double default_hdg, bool onGround);
    double calculateGain(const AircraftState& state, const AssignedValues& assigned, Point2 cur, Point2 prev, double loc_brg, bool onGround);
    double GetTrackTurnData(const AircraftState& state);
    double GetTrackSpeedData(const AircraftState& state, const DefaultValues& defaults);
    void checkRateReset(AssignedValues& assigned, DefaultValues& defaults, bool no_track);
    void checkPathHolds(ACF_STATE& aircraft_state, const AircraftState& state, const AssignedValues& assigned);
    bool arrived(AircraftState& state, AssignedValues& assigned, const DefaultValues& defaults, const PerfValues& perf);
    bool isTurnReady(const AircraftState& state, const AssignedValues& assigned, const PerfValues& perf);
    bool defaultTurnDistance(const AircraftState& state);
    bool circularDistance(const AircraftState& state, Point2* p, double distance_meters);
    void refreshRoute();
    bool doPointSkip();
    double calcSpeedForInitTurn(double turnAngle);
    
    double initialTurnAngle = -1.0;
};