#ifndef AIRCRAFT_H 
#define AIRCFAFT_H

#include <boost/date_time/posix_time/posix_time.hpp>

#include <Windows.h>
#include <vector>
#include <unordered_map>

class tcpinterface;

#include "clinc2.h"
#include "constants.h"

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
	int flightRules;
	int cycle;
	void updateFlightPlan(char* depart, char* arrive);
};

class Aircraft {
private:
	int userIndex;
	HANDLE aMutex;
	int index;
	bool renderCallsign;
	bool collision;
	bool heavy;
	std::string acfTitle;
	double latitude;
	double longitude;
	double speed;
	double heading, pitch, roll;
	int altitude;
	std::vector<History*> historyCount;
	FlightPlan* flight_plan;
	tcpinterface* intter;
	Identity* identity;
	int mode;
	std::string transponder = "0000";
	long long update_time;
	unsigned short visibility = 300;
	long long last_move = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	AV_CLIENT type;
public:
	bool connected = false;
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
	bool getRenderCallsign();
	FlightPlan* getFlightPlan();
	tcpinterface* getConnection() { return intter; }
	Identity* getIdentity() { return identity; }
	bool created, que_delete;
	void setRenderCallsign(bool);
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
	bool isCollision();
	void setCollision(bool);
	bool isHeavy();
	void setHeavy(bool);
	std::string getSquawkCode();
	void setSquawkCode(std::string value);
	int getMode();
	void setMode(int mode);
	void updateMovement();
	AV_CLIENT getType() { return type; }
	void setType(AV_CLIENT t) { type = t; }
};


extern std::unordered_map<std::string, Aircraft*> AcfMap;

extern Aircraft* getAircraftByIndex(int);

#endif