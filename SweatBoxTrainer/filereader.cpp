#include "filereader.h"

#include <fstream>
#include <sstream>
#include <stdexcept> // For std::runtime_error

#include "tools.h"
#include "aircraft/Aircraft.h" // For class definitions
#include "usermanager.h"

// These should be moved to a settings/app context class to avoid globals.
// For now, they are left but their modification logic is moved to the caller.
std::string LAST_AGC_PATH, LAST_APRT_DIR, LAST_SCT_PATH;

// --- Public Method Implementations ---

std::vector<std::unique_ptr<Aircraft>> FileReader::loadAgc(const std::string& path) {
    // 1. Reset state for a new parsing session
    m_agcResults.clear();
    m_currentAgcAircraft.reset();
    m_agcLineCounter = 0;

    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open AGC file: " + path);
    }

    // The caller is now responsible for updating LAST_AGC_PATH

    std::string line;
    std::string commentStart = ";";
    while (getline(file, line)) {
        size_t foundComment = line.find(commentStart);
        if (foundComment != std::string::npos) {
            line = line.substr(0, foundComment);
        }
        line = trim(line);

        if (!line.empty()) {
            try {
                parseAgcLine(line);
            }
            catch (...) {
                // Log or handle the error for the specific line
                // For now, we'll just skip the malformed line
                m_currentAgcAircraft.reset(); // Invalidate current aircraft
                m_agcLineCounter = 0;
            }
        }
    }
    return std::move(m_agcResults); // Return the collected aircraft
}

std::unique_ptr<Airport> FileReader::loadApt(const std::string& path) {
    // 1. Reset state
    m_currentApt.reset();
    m_currentAptPath = nullptr;
    m_currentAptApproach = nullptr;

    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr; // Indicate failure by returning a null pointer
    }

    // The caller is now responsible for updating LAST_APRT_DIR

    std::string line;
    std::string commentStart = ";";
    while (getline(file, line)) {
        size_t foundComment = line.find(commentStart);
        if (foundComment != std::string::npos) {
            line = line.substr(0, foundComment);
        }
        line = trim(line);

        if (!line.empty()) {
            try {
                parseAptLine(line);
            }
            catch (...) {
                printf("Malformed\n");
                // Skip malformed line
            }
        }
    }

    if (m_currentApt) {
        processRunways();
        printf("[Loaded Airport Data: %s]\n", m_currentApt->icao.c_str());
    }

    return std::move(m_currentApt);
}

std::unique_ptr<SectorData> FileReader::loadSct(const std::string& path) {
    // 1. Reset state for a new parsing session
    m_currentSectorData = std::make_unique<SectorData>();
    m_currentSctSection = SctSection::NONE;

    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr; // Indicate failure
    }

    // The caller is now responsible for updating LAST_SCT_PATH

    std::string line;
    std::string commentStart = ";";
    while (getline(file, line)) {
        size_t foundComment = line.find(commentStart);
        if (foundComment != std::string::npos) {
            line = line.substr(0, foundComment);
        }
        line = trim(line);

        if (!line.empty()) {
            try {
                parseSctLine(line);
            }
            catch (...) {
                // Skip malformed lines
            }
        }
    }

    return std::move(m_currentSectorData);
}

// --- Private Helper Implementations ---

void FileReader::parseAgcLine(const std::string& line) {
    if (m_agcLineCounter == 0) {
        std::vector<std::string> args = split(line, ":");
        if (args.size() < 11) throw std::runtime_error("Invalid aircraft definition line.");

        std::string squawk_mode = args[7];
        int mode = squawk_mode[0] == 'C' ? 1 : squawk_mode[0] == 'I' ? 2 : 0;
        bool heavy = args[8][0] == 'H';

        m_currentAgcAircraft = createAircraft(args[0], atodd(args[1]), atodd(args[2]), atodd(args[3]), atodd(args[4]),
            (int)atodd(args[5]), (int)atodd(args[6]), mode, args[9]);

        double initialSpeed = atodd(args[4]);
        if (initialSpeed > 0.0) {
            m_currentAgcAircraft->getDefaultValues().speed = initialSpeed;
        }
        m_currentAgcAircraft->setHeavy(heavy);
        m_currentAgcAircraft->setAptIcao(args[10]);
        m_currentAgcAircraft->getAirport();
    }
    else if (m_agcLineCounter == 1) {
        if (!m_currentAgcAircraft) return; // Skip if first line was bad
        std::vector<std::string> args = split(line, ":");
        if (args.size() < 9) throw std::runtime_error("Invalid flight plan line.");

        FlightPlan& fp = m_currentAgcAircraft->getFlightPlan();
        fp.flightRules = (args[0][0] == 'I' ? 0 : (args[0][0] == 'V' ? 1 : 2));
        fp.acType = args[1].length() > 9 ? args[1].substr(0, 8) : args[1];
        fp.departure = args[2].length() > 4 ? args[2].substr(0, 3) : args[2];
        fp.arrival = args[3].length() > 4 ? args[3].substr(0, 3) : args[3];
        fp.route = args[4].length() > 128 ? args[4].substr(0, 127) : args[4];
        fp.cruise = FormatAltitude(args[6].length() > 6 ? args[6].substr(0, 5) : args[6]);
        fp.alternate = args[7].length() > 4 ? args[7].substr(0, 3) : args[7];
        fp.remarks = args[8].length() > 128 ? args[8].substr(0, 127) : args[8];
        ++fp.cycle;
    }
    else if (m_agcLineCounter == 2) {
        if (!m_currentAgcAircraft) return; // Skip if previous lines were bad
        std::vector<std::string> args = split(line, ":");
        if (args.size() < 4) throw std::runtime_error("Invalid performance data line.");

        PerfValues& pv = m_currentAgcAircraft->getPerfValues();
        pv.takeoff_accel = atodd(args[0]);
        pv.v1 = atodd(args[1]);
        pv.climb = atodd(args[2]);
        pv.init_alt = atodd(args[3]);

        // Block is complete, move aircraft to results and reset for next block.
        m_agcResults.push_back(std::move(m_currentAgcAircraft));
    }

    // Increment and wrap counter
    m_agcLineCounter = (m_agcLineCounter + 1) % 3;
}


void FileReader::parseAptLine(const std::string& line) {
    // --- Phase 1: Look for top-level airport properties ---

    if (line.rfind("icao=", 0) == 0) { // More robust than find()
        std::string icao = split(line, "=")[1];
        capitalize(icao);

        // This is the starting point for a new airport.
        // Create the main airport object.
        m_currentApt = std::make_unique<Airport>(icao);

        // Reset sub-section pointers.
        m_currentAptPath = nullptr;
        m_currentAptApproach = nullptr;
        return;
    }

    // --- GUARD: Nothing can be processed until we have an airport context ---
    if (!m_currentApt) {
        return;
    }

    if (line.rfind("field elevation=", 0) == 0) {
        m_currentApt->elevation = atodd(split(line, "=")[1]);
        return;
    }

    // --- Phase 2: Check for a new section header like [TAXIWAY A] ---

    if (line[0] == '[' && line.back() == ']') {
        // Reset pointers, as we are starting a new section.
        m_currentAptPath = nullptr;
        m_currentAptApproach = nullptr;

        std::string content = line.substr(1, line.length() - 2);
        std::vector<std::string> header = split(content, " ");
        if (header.size() != 2) return; // Malformed header

        std::string sectionType = header[0];
        std::string sectionName = header[1];
        capitalize(sectionName);

        if (sectionType == "PARKING") {
            auto park = std::make_unique<Parking>();
            park->name = sectionName;
            m_currentAptPath = park.get(); // Set as current path (non-owning)
            m_currentApt->all[sectionName] = park.get();
            m_currentApt->parking.push_back(std::move(park)); // Transfer ownership
        }
        else if (sectionType == "TAXIWAY") {
            auto taxi = std::make_unique<Taxiway>();
            taxi->name = sectionName;
            m_currentAptPath = taxi.get();
            m_currentApt->all[sectionName] = taxi.get();
            m_currentApt->taxiway.push_back(std::move(taxi));
        }
        else if (sectionType == "RUNWAY") {
            auto rwy = std::make_unique<Runway>();
            rwy->name = sectionName;
            m_currentAptPath = rwy.get();
            m_currentApt->all[sectionName] = rwy.get();
            m_currentApt->runways.push_back(std::move(rwy));
        }
        else if (sectionType == "ILS") {
            auto ils = std::make_unique<ILS>();
            ils->name = sectionName;
            m_currentAptApproach = ils.get(); // Set as current approach (non-owning)
            m_currentApt->approaches.push_back(std::move(ils)); // Transfer ownership
        }
        else if (sectionType == "LOC") {
            auto loc = std::make_unique<LOC>();
            loc->name = sectionName;
            m_currentAptApproach = loc.get();
            m_currentApt->approaches.push_back(std::move(loc));
        }
        return;
    }

    // --- Phase 3: Process data for the current active section ---

    if (m_currentAptPath) {
        // We are inside a [PARKING], [TAXIWAY], or [RUNWAY] section
        if (m_currentAptPath->type == PATHTYPE::RUNWAY) {
            // Runway sections can have attributes or nodes
            auto* rwy = static_cast<Runway*>(m_currentAptPath);
            if (line.rfind("turnoff=", 0) == 0) {
                rwy->turnoff = split(line, "=")[1];
            }
            else if (line.rfind("displaced threshold=", 0) == 0) {
                rwy->displacement = split(line, "=")[1];
            }
            else {
                // Not an attribute, so it must be a point
                std::vector<std::string> args = split(line, " ");
                if (args.size() < 2) return; // Malformed point
                auto p = std::make_unique<Point2>(atodd(args[1]), atodd(args[0])); // x=lon, y=lat
                p->parent = m_currentAptPath;
                p->index = static_cast<int>(m_currentAptPath->nodes.size());
                m_currentAptPath->nodes.push_back(std::move(p));
            }
        }
        else {
            // Taxiway and Parking sections only have nodes
            std::vector<std::string> args = split(line, " ");
            if (args.size() < 2) return; // Malformed point
            auto p = std::make_unique<Point2>(atodd(args[1]), atodd(args[0])); // x=lon, y=lat
            p->parent = m_currentAptPath;
            p->index = static_cast<int>(m_currentAptPath->nodes.size());
            m_currentAptPath->nodes.push_back(std::move(p));
        }
    }
    else if (m_currentAptApproach) {
        // We are inside an [ILS] or [LOC] section
        if (line.rfind("heading=", 0) == 0) {
            m_currentAptApproach->h_degrees = atodd(split(line, "=")[1]);
        }
        else if (m_currentAptApproach->type == APPRTYPE::ILS && line.rfind("glideslope=", 0) == 0) {
            m_currentAptApproach->v_degrees = atodd(split(line, "=")[1]);
        }
        else {
            // Not an attribute, so it must be the approach reference point
            std::vector<std::string> args = split(line, " ");
            if (args.size() < 2) return; // Malformed point
            m_currentAptApproach->point.x_ = atodd(args[1]); // lon
            m_currentAptApproach->point.y_ = atodd(args[0]); // lat
        }
    }
}

void FileReader::parseSctLine(const std::string& line) {
    // Check for a new section header first
    if (line[0] == '[' && line.back() == ']') {
        if (line == "[VOR]") m_currentSctSection = SctSection::VOR;
        else if (line == "[NDB]") m_currentSctSection = SctSection::NDB;
        else if (line == "[AIRPORT]") m_currentSctSection = SctSection::AIRPORT;
        else if (line == "[FIXES]") m_currentSctSection = SctSection::FIXES;
        else m_currentSctSection = SctSection::NONE;
        return; // Done processing this line
    }

    // Process the line based on the current active section
    switch (m_currentSctSection) {
    case SctSection::VOR: {
        // Format: VOR_NAME FREQ LAT LON
        std::vector<std::string> args = split(line, " ");
        if (args.size() < 4) return;
        VOR vor;
        vor.name = args[0];
        vor.frequency = atodd(args[1]);
        // Assuming LAT/LON are in a specific format like N034.56.23.123 W098.12.45.678
        // This requires a more complex parser than atodd. For now, let's assume they are decimal.
        vor.latitude = atodd(args[2]);
        vor.longitude = atodd(args[3]);
        m_currentSectorData->vors.push_back(vor);
        break;
    }
    case SctSection::NDB: {
        // Format: NDB_NAME FREQ LAT LON
        std::vector<std::string> args = split(line, " ");
        if (args.size() < 4) return;
        NDB ndb;
        ndb.name = args[0];
        ndb.frequency = std::stoi(args[1]);
        ndb.latitude = atodd(args[2]);
        ndb.longitude = atodd(args[3]);
        m_currentSectorData->ndbs.push_back(ndb);
        break;
    }
    case SctSection::FIXES: {
        // Format: FIX_NAME LAT LON
        std::vector<std::string> args = split(line, " ");
        if (args.size() < 3) return;
        Fix fix;
        fix.name = args[0];
        fix.latitude = atodd(args[2]); // Note: VRC format is often NAME LAT LON
        fix.longitude = atodd(args[3]);
        m_currentSectorData->fixes.push_back(fix);
        break;
    }
    case SctSection::AIRPORT:
    case SctSection::NONE:
    default:
        // Do nothing for these sections for now
        break;
    }
}


void FileReader::processRunways() {
    if (!m_currentApt) return;

    std::vector<std::unique_ptr<Runway>> newRunways;
    newRunways.reserve(m_currentApt->runways.size() * 2); // Pre-allocate

    auto processRunway = [this, &newRunways](auto& rwy_ptr) {
        const auto names = split(rwy_ptr->name, "/");

        // Handle single-name runways immediately
        if (names.size() != 2) {
            newRunways.push_back(std::move(rwy_ptr));
            return;
        }

        // Validate names BEFORE modifying state
        if (m_currentApt->all.contains(names[0]) ||
            m_currentApt->all.contains(names[1])) {
            // Handle duplicate names (log error in real implementation)
            newRunways.push_back(std::move(rwy_ptr));
            return;
        }

        // 1. Process primary runway (second name)
        const std::string primaryName = names[0];
        auto& primaryRwy = rwy_ptr;
        primaryRwy->name = primaryName;
        m_currentApt->all[primaryName] = primaryRwy.get();

        // 2. Create reverse runway (first name)
        auto reverseRwy = std::make_unique<Runway>();
        reverseRwy->name = names[1];
        reverseRwy->nodes.reserve(primaryRwy->nodes.size());

        // Efficient point transfer with index correction
        for (int i = 0; i < primaryRwy->nodes.size(); ++i) {
            const int reverseIndex = primaryRwy->nodes.size() - 1 - i;
            auto& srcPoint = primaryRwy->nodes[reverseIndex];

            auto newPoint = std::make_unique<Point2>(*srcPoint);
            newPoint->parent = reverseRwy.get();
            newPoint->index = i;  // CORRECT sequential index

            reverseRwy->nodes.push_back(std::move(newPoint));
        }

        // Atomic map update AFTER runway is fully constructed
        m_currentApt->all[reverseRwy->name] = reverseRwy.get();

        // Add both runways (reverse first for logical ordering)
        newRunways.push_back(std::move(reverseRwy));
        newRunways.push_back(std::move(primaryRwy));
        };

    // Process all runways with exception safety
    for (auto& rwy : m_currentApt->runways) {
        processRunway(rwy);
    }

    m_currentApt->runways = std::move(newRunways);
}
