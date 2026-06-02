#pragma once

#include <string>

// Forward-declare the Aircraft class to avoid circular includes
class Aircraft;

namespace CommandHandlers {

    // This is the one-time setup function to populate the command map.
    // Call this at application startup.
    void initialize();

    // This is the main entry point for processing a command for a specific aircraft.
    void processCommand(Aircraft& aircraft, const std::string& commandString);

} // namespace CommandHandlers