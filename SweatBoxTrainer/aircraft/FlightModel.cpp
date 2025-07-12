#include "FlightModel.h"
#include "../tools.h"
#include "Aircraft.h"

void FlightModel::updateState(AircraftState& state, const AssignedValues& assigned, const PerfValues& perf, const DefaultValues& defaults, bool onGround, bool isHolding, bool isTakeoff) {
    long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    
    // Initialize timers on first run
    if (last_time_speed == 0) {
        last_time_speed = last_time_roll = last_time_pitch = last_time_heading = last_time_movement = last_time_altitude = now;
    }

    double new_speed = getNextSpeed(state.speed, assigned, perf, onGround, isHolding, isTakeoff, (double)(now - last_time_speed));
    if (state.speed != new_speed) {
        state.speed = new_speed;
        state.MarkDirty(AircraftDirtyFlags::SPEED);
    }
    last_time_speed = now;

    double new_roll = getNextRoll(state.roll, assigned, perf, (double)(now - last_time_roll));
    if (state.roll != new_roll) {
        state.roll = new_roll;
        state.MarkDirty(AircraftDirtyFlags::ROLL);
    }
    last_time_roll = now;
    
    double new_pitch = getNextPitch(state.pitch, assigned, perf, (double)(now - last_time_pitch));
    if (state.pitch != new_pitch) {
        state.pitch = new_pitch;
        state.MarkDirty(AircraftDirtyFlags::PITCH);
    }
    last_time_pitch = now;

    double new_heading = getNextHeading(state.heading, state.roll, state.speed, assigned, -1, onGround, (double)(now - last_time_heading));
    if (state.heading != new_heading) {
        state.heading = new_heading;
        state.MarkDirty(AircraftDirtyFlags::HEADING);
    }
    if ((state.heading == assigned.asgd_heading) && state.roll != 0) {
        // This is a bit of a hacky dependency. A better state machine would handle this.
        // For now, we'll leave it but acknowledge it's not ideal.
        // assigned.asdg_roll = 0; // The flight model should NOT modify assigned values.
    }
    last_time_heading = now;
    
    updateMovement(state, (double)(now - last_time_movement));
    last_time_movement = now;

    double new_altitude = getNextAltitude(state.altitude, state.verticalSpeed, assigned, (double)(now - last_time_altitude));
    if (state.altitude != new_altitude) {
        state.altitude = new_altitude;
        state.MarkDirty(AircraftDirtyFlags::ALTITUDE);
    }
    last_time_altitude = now;
}

void FlightModel::updateMovement(AircraftState& state, double interval_ms)
{
    double dist = get_distance(state.speed, (long long)interval_ms);
    Point2 p = getLocFromBearing(state.latitude, state.longitude, dist, state.heading);
    if (p.x_ != state.longitude) {
        state.longitude = p.x_;
        state.MarkDirty(AircraftDirtyFlags::LONGITUDE);
    }
    if (p.y_ != state.latitude) {
        state.latitude = p.y_;
        state.MarkDirty(AircraftDirtyFlags::LATITUDE);
    }
}

double FlightModel::getNextSpeed(double current_speed, const AssignedValues& assigned, const PerfValues& perf, bool onGround, bool isHolding, bool isTakeoff, double interval_ms)
{
    double next_speed = current_speed;
    double a_spd = ((onGround && isHolding) ? 0 : assigned.asdg_speed);

    if (current_speed != a_spd)
    {
        double spd_delta = a_spd - current_speed;
        double acceleration = onGround ? (spd_delta < 0 ? assigned.asdg_gnd_braking :
            (isTakeoff ? perf.takeoff_accel : assigned.asdg_gnd_accel))
            : assigned.asdg_accel;
        double amount = get_per_second(acceleration, interval_ms);

        if (spd_delta != 0)
        {
            if (spd_delta < 0) {
                if ((current_speed - amount) <= a_spd)
                    next_speed = a_spd;
                else
                    next_speed -= amount;
            }
            else if (spd_delta > 0)
            {
                if ((current_speed + amount) >= a_spd)
                    next_speed = a_spd;
                else
                    next_speed += amount;
            }
        }
    }
    return next_speed;
}

double FlightModel::getNextHeading(double current_heading, double current_roll, double current_speed, const AssignedValues& assigned, int turnOri, bool onGround, double interval_ms)
{
    double next_hdg = current_heading;
    double amount = onGround ? (assigned.asdg_gnd_turn_rate * (interval_ms / 1000.0)) : get_rot(current_roll, current_speed, (long long)interval_ms);
    double new_hdg = get_angle_unsigned(hdg(assigned.asgd_heading), hdg(current_heading));
    double a_hdg = assigned.asgd_heading;
    if (current_speed != 0)
    {
        if (current_heading != a_hdg)
        {
            int turnOrientation = onGround ? -1 : turnOri;
            if (turnOrientation == -1)
            {
                if (new_hdg > 0)
                {
                    if (get_angle_unsigned(a_hdg, hdg((current_heading + amount))) <= 0)
                        next_hdg = a_hdg;
                    else
                        next_hdg = hdg(current_heading + amount);
                }
                else if (new_hdg < 0)
                {
                    if (get_angle_unsigned(a_hdg, hdg((current_heading - amount))) >= 0)
                        next_hdg = a_hdg;
                    else
                        next_hdg = hdg(current_heading - amount);
                }
            }
            else if (turnOrientation == 0)//left turn
            {
                if (get_angle_unsigned(a_hdg, hdg((current_heading - amount))) >= 0)
                    next_hdg = a_hdg;
                else
                    next_hdg = hdg(current_heading - amount);
            }
            else // right turn
            {
                if (get_angle_unsigned(a_hdg, hdg((current_heading + amount))) <= 0)
                    next_hdg = a_hdg;
                else
                    next_hdg = hdg(current_heading + amount);
            }
        }
    }

    return next_hdg;
}

double FlightModel::getNextPitch(double current_pitch, const AssignedValues& assigned, const PerfValues& perf, double interval_ms)
{
    double next_pitch = current_pitch;
    double amount = get_per_second(perf.pitch_rate, interval_ms);

    double a_pitch = assigned.asdg_pitch;
    double pitch_delta = a_pitch - current_pitch;

    if (pitch_delta < 0)
    {
        if ((next_pitch - amount) < a_pitch)
            next_pitch = a_pitch;
        else
            next_pitch -= amount;
    }
    else if (pitch_delta > 0)
    {
        if ((next_pitch + amount) > a_pitch)
            next_pitch = a_pitch;
        else
            next_pitch += amount;
    }

    return next_pitch;
}

double FlightModel::getNextRoll(double current_roll, const AssignedValues& assigned, const PerfValues& perf, double interval_ms)
{
    double next_roll = current_roll;
    double amount = get_per_second(perf.roll_rate, interval_ms);

    double a_roll = assigned.asdg_roll;
    double roll_delta = a_roll - current_roll;

    if (roll_delta < 0)
    {
        if ((next_roll - amount) < a_roll)
            next_roll = a_roll;
        else
            next_roll -= amount;
    }
    else if (roll_delta > 0)
    {
        if ((next_roll + amount) > a_roll)
            next_roll = a_roll;
        else
            next_roll += amount;
    }

    return next_roll;
}

double FlightModel::getNextAltitude(double current_altitude, double current_vspeed, const AssignedValues& assigned, double interval_ms)
{
    double next_alt = current_altitude;
    double amount = get_per_minute(current_vspeed, interval_ms);

    double a_alt = assigned.asdg_altitude;
    double alt_delta = a_alt - current_altitude;

    if (alt_delta < 0)
    {
        if ((next_alt - amount) < a_alt)
            next_alt = a_alt;
        else
            next_alt -= amount;
    }
    else if (alt_delta > 0)
    {
        if ((next_alt + amount) > a_alt)
            next_alt = a_alt;
        else
            next_alt += amount;
    }

    return next_alt;
}