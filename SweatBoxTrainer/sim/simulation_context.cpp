#include "simulation_context.h"

SimulationContext& SimulationContext::instance() {
    static SimulationContext ctx;
    return ctx;
}

// NOTE: The caller MUST hold a lock on aircraftMutex().
Aircraft* SimulationContext::findAircraftByIndex(int index) {
    auto it_idx = m_indexToCallsign.find(index);
    if (it_idx == m_indexToCallsign.end()) {
        return nullptr; // No callsign for this index
    }
    const std::string& callsign = it_idx->second;

    auto it_acf = m_aircraft.find(callsign);
    if (it_acf == m_aircraft.end()) {
        // This indicates an inconsistency, the index map has a callsign
        // that doesn't exist in the aircraft map. Should be logged.
        return nullptr;
    }
    return it_acf->second.get();
}