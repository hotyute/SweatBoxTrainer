#pragma once


#define MAX_EVENTS 1536
#define MAX_AIRCRAFT_SIZE 1024

enum class AV_CLIENT { CONTROLLER = 0, PILOT = 1 };

enum class ACF_STATE {IDLE, HOLDING, TAXING, TAKEOFF, LANDING, AIRBORNE};

const long long CALC_TIME = 30L;

const double KNOTS_KM = 1.852;

const double KNOTS_FT = 6076.1200001180986874;

const double NM_PER_DEG = 60.0;

const double DEG_PER_NM = 1.0 / NM_PER_DEG;

const double DEFAULT_TURN_RATE = 10.0;

const int TURN_RATE_TAXI_MIN = 5;       // Degrees per second.
const int TURN_RATE_TAXI = 20;          // Degrees per second.
const int SPEED_MIN = 10;
const int SPEED_MAX = 20;