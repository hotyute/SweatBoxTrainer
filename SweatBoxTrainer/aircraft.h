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

struct AssignedValues {
	double asgd_heading = 0;
	int asdg_altitude = 0;
	double asdg_speed = 0;
	double asdg_roll = 25;
	double asdg_gnd_turn_rate = 9; // 9 degrees per second
	double asdg_accel = 2;//per second
	double asdg_gnd_accel = 0.5;//per second
};

class History {
};

struct Identity {
	std::string callsign;
	std::string login_name;
	std::string password;
	std::string username;
	int id;
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
	int userIndex;
	HANDLE aMutex;
	int index;
	bool heavy;
	std::string acfTitle;
	double latitude;
	double longitude;
	double speed;
	double heading, pitch, roll;
	int altitude = 0, verticalSpeed = 1000;
	std::vector<History*> historyCount;
	FlightPlan flight_plan;
	tcpinterface intter = tcpinterface(this);
	Identity identity;
	AssignedValues assignedValues;
	int mode;
	std::string transponder = "0000";
	long long update_time;
	unsigned short visibility = 300;
	long long last_time[4];
	AV_CLIENT type;
public:
	int turnOri = -1;
	std::string apt_icao = "";
	bool connected = false, flying_course = false;
	std::vector<std::string> ground_route;
	Point2* ground_prev = nullptr, * ground_cur = nullptr, * taxiway_end = nullptr, * future_point = nullptr;
	TaxiPath* cur_path = nullptr, * next_path = nullptr, * next_next_path = nullptr;
	Point2* air_prev = nullptr, * air_cur = nullptr;
	Aircraft();
	~Aircraft();
	const int getUserIndex() const { return userIndex; }
	void setUserIndex(int index) { userIndex = index; }
	long long getUpdateTime() { return update_time; }
	void setUpdateTime(long long value) { update_time = value; }
	void setVisibility(unsigned short vis) { this->visibility = vis; }
	unsigned short getVisibility() { return this->visibility; }
	unsigned int Ccallsign;
	int getIndex();
	void setIndex(int);
	FlightPlan& getFlightPlan() { return flight_plan; }
	tcpinterface& getConnection() { return intter; }
	Identity* getIdentity() { return &identity; }
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
	double getNextSpeed(double interval_ms);
	double getNextHeading(double interval_ms);
	double getNextPoint();
	void processRoute();
	bool arrived(Point2* p2);
	bool arrived(Point2* p1, Point2* p2);
	AssignedValues& getAssignedValues() { return assignedValues; }
	double calculateGS(double __unnamed000, double __unnamed001, double gs_angle, double dest_altitude);
	double calculateLoc(double __unnamed000, double __unnamed001, double __unnamed002, double destHdg);
	double calculateGain(Point2 cur, Point2 prev, double __unnamed002);
	double calculateTurnDistance(Point2* p, Point2* p2);
	bool isTurnReady(Point2* p, Point2* p2);
	bool isTurnReady(Point2* p, Point2* p2, double distance);
	bool defaultTurnDistance();
	bool defaultTurnDistance(Point2* p1);
	Point2* getFuturePoint();
	Point2** checkEarlyTurn(bool is_early);
};


extern std::unordered_map<std::string, Aircraft*> AcfMap;

extern Aircraft* getAircraftByIndex(int);

void addUserToLB(Aircraft* user);

void DisplayAircraft();

#endif