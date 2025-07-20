#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>
#include "../aircraft/aircraft.h"
#include "../airport.h"

class SimulationContext {
public:
    static SimulationContext& instance();          // singleton

    // aircraft -------------------------------------------------------------
    using AircraftMap = std::unordered_map<std::string, std::unique_ptr<Aircraft>>;
    AircraftMap& aircraft() { return m_aircraft; }
    std::mutex& aircraftMutex() { return m_aircraftMutex; }

    // airports -------------------------------------------------------------
    using AirportMap = std::unordered_map<std::string, std::unique_ptr<Airport>>;
    AirportMap& airports() { return m_airports; }

private:
    SimulationContext() = default;
    AircraftMap  m_aircraft;
    AirportMap   m_airports;
    std::mutex   m_aircraftMutex;
};