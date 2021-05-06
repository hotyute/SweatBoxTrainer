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
	double asdg_altitude = 0;
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
	int controller_rating;
	int pilot_rating;
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
	bool connected = false;
	std::vector<Point2*> ground_route;
	std::vector<Point2*> air_route;
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
	void taxi(Airport* airport, std::string hs, std::vector<std::string>& s);
	bool onGround();
	AssignedValues& getAssignedValues() { return assignedValues; }
	double getNextHeading();
};


extern std::unordered_map<std::string, Aircraft*> AcfMap;

extern Aircraft* getAircraftByIndex(int);

void addUserToLB(Aircraft* user);

void DisplayAircraft();

#endif