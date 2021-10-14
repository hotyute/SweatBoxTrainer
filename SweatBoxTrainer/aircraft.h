#ifndef AIRCRAFT_H 
#define AIRCFAFT_H

#include <boost/date_time/posix_time/posix_time.hpp>

#include <Windows.h>
#include <vector>
#include <unordered_map>

class tcpinterface;

#include "clinc2.h"
#include "constants.h"
#include "airport.h"
#include "events.h"

struct DefaultValues {
	double speed = 15;
	double turn_rate = 10;
};

struct AssignedValues {
	double asgd_heading = 0;
	int asdg_altitude = 0;
	double asdg_speed = 0;
	double asdg_roll = 25;
	double asdg_gnd_turn_rate = 10; // 9 degrees per second
	double asdg_accel = 5;//per second
	double asdg_gnd_accel = 2;//per second
	double asdg_to_accel = 5;//per second
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
private:
	Airport* airport = nullptr;
	int userIndex = -1;
	HANDLE aMutex;
	int index = -1;
	bool heavy = false;
	std::string acfTitle;
	double latitude;
	double longitude;
	double speed = 0;
	double heading, pitch, roll;
	int altitude = 0, verticalSpeed = 1000;
	std::vector<History*> historyCount;
	FlightPlan flight_plan;
	tcpinterface intter = tcpinterface(this);
	Identity identity;
	AssignedValues assignedValues;
	DefaultValues defaultValues;
	int mode = 0;
	std::string transponder = "0000";
	long long update_time = 0;
	unsigned short visibility = 300;
	long long last_time[4];
	AV_CLIENT type = AV_CLIENT::PILOT;
public:
	Aircraft* HoldingFor = nullptr;
	Runway* runway_ctx = nullptr;
	int turnOri = -1;
	std::string apt_icao = "";
	Event* position_updates = new PositionUpdates(this);
	ACF_STATE state = ACF_STATE::IDLE;
	bool connected = false, point_skip = false, locked_rate = false, queue_takeoff = false;
	std::vector<std::string> ground_route;
	std::vector<Point2*> ground_points, holds;
	Point2* ground_prev = nullptr, * ground_cur = nullptr, * ground_next = nullptr, * ground_next_next = nullptr;
	Point2* air_prev = nullptr, * air_cur = nullptr;
	Aircraft();
	~Aircraft();
	const int getUserIndex() const { return userIndex; }
	void setUserIndex(int index) { userIndex = index; }
	long long getUpdateTime() { return update_time; }
	void setUpdateTime(long long value) { update_time = value; }
	void setVisibility(unsigned short vis) { this->visibility = vis; }
	unsigned short getVisibility() { return this->visibility; }
	int getIndex();
	void setIndex(int);
	FlightPlan& getFlightPlan() { return flight_plan; }
	tcpinterface& getConnection() { return intter; }
	Identity* getIdentity() { return &identity; }
	std::string getCallSign() { return identity.callsign; }
	bool created, que_delete;
	std::string getAcfTitle();
	void setAcfTitle(std::string);
	double getLatitude();
	void setLatitude(double);
	double getLongitude();
	void setLongitude(double);
	int getAltitude();
	void setAltitude(int);
	double getSpeed();
	void setSpeed(double);
	double getHeading();
	void setHeading(double);
	double getPitch();
	void setPitch(double);
	double getRoll();
	void setRoll(double);
	HANDLE getMutex();
	void lock();
	void unlock();
	void set_holding() { state = ACF_STATE::HOLDING; }
	bool holding() { return state == ACF_STATE::HOLDING; }
	void set_taxing() { state = ACF_STATE::TAXING; }
	bool taxing() { return state == ACF_STATE::TAXING; };
	bool takeoff() { return state == ACF_STATE::TAKEOFF; };
	bool idle() { return state == ACF_STATE::IDLE; };
	//void setUser1(User*);
	bool isHeavy();
	void setHeavy(bool);
	std::string getSquawkCode();
	void setSquawkCode(std::string value);
	int getMode();
	void setMode(int mode);
	void updateSpeed();
	void updateHeading();
	void updateMovement();
	AV_CLIENT getType() { return type; }
	void setType(AV_CLIENT t) { type = t; }
	int getVerticalSpeed() { return verticalSpeed; }
	void setVerticalSpeed(int vs) { verticalSpeed = vs; }
	Airport* getAirport();
	bool onGround();
	double GetTrackTurnData();
	double GetTrackSpeedData();
	bool OnTrack();
	double getNextSpeed(double interval_ms);
	double getNextHeading(double interval_ms);
	void checkRateReset(bool no_track);
	double getNextPoint();
	void pollRoute();
	void refreshRoute();
	void prepareRoute();
	bool checkTooShort();
	bool arrived(Point2* p2);
	bool arrived(Point2* p1, Point2* p2);
	void reset_path();
	AssignedValues& getAssignedValues() { return assignedValues; }
	DefaultValues& getDefaultValues() { return defaultValues; }
	double calculateGS(double __unnamed000, double __unnamed001, double gs_angle, double dest_altitude);
	double calculateLoc(double __unnamed000, double __unnamed001, double __unnamed002, double destHdg);
	double calculateGain(Point2 cur, Point2 prev, double __unnamed002);
	double calculateTurnDistance(Point2* p, Point2* p2);
	bool isTurnReady(Point2* p, Point2* p2);
	bool isTurnReady(Point2* p, Point2* p2, double distance);
	bool defaultTurnDistance();
	bool circularDistance(Point2* p1, double distance_meters);
	Point2* getFuturePoint(TaxiPath* cur_path, TaxiPath* next_path, Point2* taxiway_end);
	bool doPointSkip();
	void CollisionDetection();
	void checkPathHolds();
	Point2 GetCurLoc();
	Point2 GetNextLoc();
	void HoldAt(std::string s);
	void handle_takeoff();
};


extern std::unordered_map<std::string, Aircraft*> AcfMap;

extern Aircraft* getAircraftByIndex(int);

void addUserToLB(Aircraft* user);

void DisplayAircraft();

#endif