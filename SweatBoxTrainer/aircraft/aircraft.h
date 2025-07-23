#pragma once

#include <unordered_map>
#include <vector>
#include <Windows.h>
#include <string>
#include <memory>

#include "FlightModel.h"
#include "RouteManager.h"

#include "AircraftState.h"
#include "../airport.h"
#include "../constants.h"
#include "../clinc2.h"
#include "AircraftPingTask.h"

class AircraftPositionUpdateTask;
class RouteManager;
class FlightModel;

struct DefaultValues {
    double speed = 15;
    double turn_rate = 10;
};

struct PerfValues {
    double v1 = 120;
    double climb = 200;
    double init_alt = 5000;
    double takeoff_accel = 5;//per second
    double roll_rate = 5;
    double pitch_rate = 5;
    double max_roll = 30;
    double max_pitch_up = 15;
    double max_pitch_down = 10;
};

struct AssignedValues {
    double asgd_heading = 0;
    double asdg_altitude = 0;
    double asdg_speed = 0;
    double asdg_roll = 0;
    double asdg_pitch = 0;
    double asdg_gnd_turn_rate = 10; // 9 degrees per second
    double asdg_accel = 5;//per second
    double asdg_gnd_accel = 2;//per second
    double asdg_gnd_braking = 5;
};

class History {
};

struct Identity {
    std::string callsign;
    std::string login_name;
    std::string password;
    std::string username;
    int id = 0;
    int controller_rating = 0, controller_position = 0;
    int pilot_rating = 0;
};

class FlightPlan {
public:
    FlightPlan();
    std::string departure, arrival, alternate, squawkCode;
    std::string acType, scratchPad, cruise, route, remarks;
    int flightRules = 0;
    int cycle = 0;
    void updateFlightPlan(char* depart, char* arrive);
};

class Aircraft {
public:
    Aircraft();
    ~Aircraft();

    // --- High-level update methods ---
    void update(); // This will be the main entry point called from the simulation loop

    void startPositionUpdates(ThreadPool& pool);
    void stopPositionUpdates();
    void startPingUpdates(ThreadPool& pool);
    void stopPingUpdates();

    void disconnect(bool queue);

    // --- Accessors for components and data ---
    AircraftState& getState() { return m_state; }
    const AircraftState& getState() const { return m_state; }
    tcp_manager& getConnection() { return m_connection; }
    Identity* getIdentity() { return &m_identity; }
    FlightPlan& getFlightPlan() { return m_flightPlan; }
    AssignedValues& getAssignedValues() { return m_assignedValues; }
    PerfValues& getPerfValues() { return m_perfValues; }
    DefaultValues& getDefaultValues() { return m_defaultValues; }
    RouteManager& getRouteManager() { return m_routeManager; }

    // --- Simple Getters/Setters ---
    std::string getCallSign() const { return m_identity.callsign; }
    int getUserIndex() const { return userIndex; }
    void setUserIndex(int index) { userIndex = index; }
    long long getUpdateTime() const { return update_time_interval_ms; }
    void setUpdateTime(long long value) { update_time_interval_ms = value; }
    void setVisibility(unsigned short vis) { this->visibility = vis; }
    unsigned short getVisibility() const { return this->visibility; }
    std::string getAcfTitle() const { return acfTitle; }
    void setAcfTitle(std::string title) { acfTitle = title; }
    bool isHeavy() const { return heavy; }
    void setHeavy(bool is_h) { heavy = is_h; }
    std::string getSquawkCode() const { return transponder; }
    void setSquawkCode(std::string value) { transponder = value; }
    int getMode() const { return mode; }
    void setMode(int newMode);
    void setVerticalSpeed(double vs);
    Airport* getAirport();
    bool onGround() const;
    AV_CLIENT getType() const { return type; }
    void setType(AV_CLIENT t) { type = t; }
    void setAptIcao(std::string icao) { apt_icao = icao; }

    // --- State Machine ---
    ACF_STATE state = ACF_STATE::IDLE;
    bool connected = false;
    int turnOri = -1;
    int frequency[2];

private:
    // --- Composed Components ---
    AircraftState m_state;
    FlightModel m_flightModel;
    RouteManager m_routeManager;
    tcp_manager m_connection;

    // --- Data Members ---
    Identity m_identity;
    FlightPlan m_flightPlan;
    AssignedValues m_assignedValues;
    DefaultValues m_defaultValues;
    PerfValues m_perfValues;
    Airport* airport = nullptr;

    // --- Other state ---
    int userIndex = -1;
    bool heavy = false;
    std::string acfTitle;
    int mode = 0;
    std::string transponder = "0000";
    std::string apt_icao = "";
    long long update_time_interval_ms = 0;
    unsigned short visibility = 300;
    AV_CLIENT type = AV_CLIENT::PILOT;

    // --- Private Methods ---
    void CollisionDetection();
    void CheckFrameFlags();
    Point2 GetNextLoc();
    void handle_takeoff_roll();
    void handle_takeoff_rotate();
    void pass_standard_pitch(double altitude);

    std::unique_ptr<AircraftPositionUpdateTask> m_positionUpdateTask;
    std::unique_ptr<AircraftPingTask> m_pingTask;

    // Friend classes if necessary, though it's better to use public interfaces
};

void addUserToLB(Aircraft* user);
void DisplayAircraft();