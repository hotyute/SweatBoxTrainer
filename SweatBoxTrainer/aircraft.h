#ifndef AIRCRAFT_H 
#define AIRCFAFT_H

#include <Windows.h>
#include <iostream>
#include <vector>
#include <unordered_map>

#include "clinc2.h"

#ifndef History_aircraft_h
#define History_aircraft_h
class History {
};
#endif

#ifndef FlightPlan_aircraft_h
#define FlightPlan_aircraft_h
class FlightPlan {
public:
	FlightPlan();
	std::string departure, arrival, alternate, squawkCode;
	std::string acType, scratchPad, cruise, route, remarks;
	int flightRules;
	int cycle;
	void updateFlightPlan(char* depart, char* arrive);
};
#endif

#ifndef Aircraft_aircraft_h
#define Aircraft_aircraft_h
class Aircraft;
class Aircraft {
private:
	int userIndex;
	HANDLE aMutex;
	int index;
	bool renderCallsign;
	bool collision;
	bool heavy;
	std::string callsign;
	std::string acfTitle;
	double latitude;
	double longitude;
	double speed;
	double heading, pitch, roll;
	int altitude;
	std::vector<History*> historyCount;
	FlightPlan* flight_plan;
	int mode;
	std::string transponder = "0000";
	tcpinterface* intter;
	long long update_time;
	unsigned short visibility = 300;
public:
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
	bool created, que_delete;
	void setRenderCallsign(bool);
	std::string getCallsign();
	void setCallsign(std::string);
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
};
#endif


extern std::unordered_map<std::string, Aircraft*> AcfMap;

extern Aircraft* getAircraftByIndex(int);

#endif