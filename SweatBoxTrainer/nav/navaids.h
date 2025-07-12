#pragma once

#include <string>
#include <vector>
#include <memory>

// A simple structure for a navigational fix.
struct Fix {
    std::string name;
    double latitude;
    double longitude;
};

// A structure for a VOR.
struct VOR {
    std::string name;
    double frequency;
    double latitude;
    double longitude;
};

// A structure for an NDB.
struct NDB {
    std::string name;
    int frequency;
    double latitude;
    double longitude;
};

// This class will hold all the data parsed from an SCT file.
class SectorData {
public:
    std::vector<Fix> fixes;
    std::vector<VOR> vors;
    std::vector<NDB> ndbs;
    // You can add other sections here as needed (airports, etc.)
};