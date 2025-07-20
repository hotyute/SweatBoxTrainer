#pragma once

#include <string>
#include <vector>
#include <memory>
#include "nav/navaids.h"
#include "airport.h"

// Forward declarations
class Aircraft;
class Airport;

class FileReader {
public:
    FileReader() = default;

    std::vector<std::unique_ptr<Aircraft>> loadAgc(const std::string& path);
    std::unique_ptr<Airport> loadApt(const std::string& path);
    std::unique_ptr<SectorData> loadSct(const std::string& path);

private:
    // Enum to track the current section in an SCT file
    enum class SctSection { NONE, VOR, NDB, AIRPORT, FIXES };

    // --- State for parsing a single AGC file ---
    std::vector<std::unique_ptr<Aircraft>> m_agcResults;
    std::unique_ptr<Aircraft> m_currentAgcAircraft;
    int m_agcLineCounter = 0;

    // --- State for parsing a single APT file ---
    std::unique_ptr<Airport> m_currentApt;
    TaxiPath* m_currentAptPath = nullptr;       // Non-owning pointer to the current path being built
    ApproachPath* m_currentAptApproach = nullptr; // Non-owning pointer to the current approach being built.

    // --- State for parsing a single SCT file ---
    std::unique_ptr<SectorData> m_currentSectorData;
    SctSection m_currentSctSection = SctSection::NONE;

    // --- Private parsing helpers ---
    void parseAgcLine(const std::string& line);
    void parseAptLine(const std::string& line);
    void parseSctLine(const std::string& line); // New helper for SCT
    void processRunways();

    static std::unique_ptr<Aircraft> createAircraftFromParams(const std::string& callsign, double latitude, double longitude, double heading, double speed, double altitude, double verticalSpeed, int mode, const std::string& squawkCode);
};

// Global settings are now managed elsewhere (e.g., an AppSettings class/singleton)
extern std::string LAST_AGC_PATH, LAST_APRT_DIR, LAST_SCT_PATH;