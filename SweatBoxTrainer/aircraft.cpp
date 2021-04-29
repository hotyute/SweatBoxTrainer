#include "aircraft.h"

#include <iostream>

Aircraft::Aircraft() {
	Aircraft::aMutex = CreateMutex(NULL, FALSE, L"Aircraft Mutex");
	Aircraft::created = false;
	Aircraft::que_delete = false;
	Aircraft::altitude = 0;
	Aircraft::pitch = 0;
	Aircraft::latitude = 0;
	Aircraft::longitude = 0;
	Aircraft::heading = 0;
	Aircraft::roll = 0;
	Aircraft::flight_plan = new FlightPlan();
	//Aircraft::intter = new tcpinterface();
}

Aircraft::~Aircraft()
{
	delete flight_plan;
}

std::unordered_map<std::string, Aircraft*>AcfMap;

int Aircraft::getIndex() {
	return Aircraft::index;
}

void Aircraft::setIndex(int value) {
	Aircraft::index = value;
}

bool Aircraft::getRenderCallsign() {
	return Aircraft::renderCallsign;
}

FlightPlan* Aircraft::getFlightPlan()
{
	return Aircraft::flight_plan;
}

void Aircraft::setRenderCallsign(bool value) {
	Aircraft::renderCallsign = value;
}

std::string Aircraft::getCallsign() {
	return Aircraft::callsign;
}

void Aircraft::setCallsign(std::string value) {
	Aircraft::callsign = value;
}

std::string Aircraft::getAcfTitle() {
	return Aircraft::acfTitle;
}

void Aircraft::setAcfTitle(std::string value) {
	Aircraft::acfTitle = value;
}

double Aircraft::getLatitude() {
	return Aircraft::latitude;
}

void Aircraft::setLatitude(double value) {
	Aircraft::latitude = value;
}

double Aircraft::getLongitude() {
	return Aircraft::longitude;
}

void Aircraft::setLongitude(double value) {
	Aircraft::longitude = value;
}

int Aircraft::getAltitude() {
	return Aircraft::altitude;
}

void Aircraft::setAltitude(int val) {
	Aircraft::altitude = val;
}

double Aircraft::getSpeed() {
	return Aircraft::speed;
}

void Aircraft::setSpeed(double value) {
	Aircraft::speed = value;
}

double Aircraft::getHeading() {
	return Aircraft::heading;
}

void Aircraft::setHeading(double value) {
	Aircraft::heading = value;
}

double Aircraft::getPitch() {
	return Aircraft::pitch;
}

void Aircraft::setPitch(double val) {
	Aircraft::pitch = val;
}

double Aircraft::getRoll() {
	return Aircraft::roll;
}

void Aircraft::setRoll(double val) {
	Aircraft::roll = val;
}

HANDLE Aircraft::getMutex() {
	return Aircraft::aMutex;
}

void Aircraft::lock() {
	WaitForSingleObject(Aircraft::aMutex, INFINITE);
}

void Aircraft::unlock() {
	ReleaseMutex(Aircraft::aMutex);
}

Aircraft* getAircraftByIndex(int index) {
	return NULL;
}

//void Aircraft::setUser1(User *value) {
//Aircraft::acfUser = value;
//}

bool Aircraft::isCollision() {
	return Aircraft::collision;
}

void Aircraft::setCollision(bool value) {
	Aircraft::collision = value;
}

bool Aircraft::isHeavy() {
	return Aircraft::heavy;
}

void Aircraft::setHeavy(bool value) {
	Aircraft::heavy = value;
}

std::string Aircraft::getSquawkCode() {
	return Aircraft::transponder;
}

void Aircraft::setSquawkCode(std::string value) {
	Aircraft::transponder = value;
}

int Aircraft::getMode() {
	return Aircraft::mode;
}

void Aircraft::setMode(int mode) {
	Aircraft::mode = mode;
}

FlightPlan::FlightPlan()
{
	FlightPlan::squawkCode = "0000";
	FlightPlan::departure = "";
	FlightPlan::arrival = "";
	FlightPlan::alternate = "";
	FlightPlan::squawkCode = "";
	FlightPlan::acType = "";
	FlightPlan::scratchPad = "";
	FlightPlan::cruise = "";
	FlightPlan::route = "";
	FlightPlan::remarks = "";
}

void FlightPlan::updateFlightPlan(char* depart, char* arrive)
{
	FlightPlan::squawkCode = "0000";
	FlightPlan::departure = depart;
	FlightPlan::arrival = arrive;
	FlightPlan::cycle++;
}
