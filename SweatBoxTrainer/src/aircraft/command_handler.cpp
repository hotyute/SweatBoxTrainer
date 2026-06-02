// --- START OF NEW FILE command_handler.cpp ---

#include "SweatBoxTrainer/aircraft/command_handler.h"

#include <unordered_map>
#include <functional>
#include <vector>
#include <string>
#include <algorithm> // for std::transform

// Include all necessary headers for command actions
#include "SweatBoxTrainer/aircraft/aircraft.h"
#include "SweatBoxTrainer/aircraft/RouteManager.h"
#include "SweatBoxTrainer/tools.h"
#include "SweatBoxTrainer/guidialogue.h"
#include "SweatBoxTrainer/packets_out.h"

namespace CommandHandlers {

    // Define the function signature for a command handler for readability
    using HandlerFunc = std::function<void(Aircraft&, const std::vector<std::string>&)>;

    // The dispatch table. 'static' keeps it private to this file.
    static std::unordered_map<std::string, HandlerFunc> s_commandHandlers;

    void ensureTaxiSpeed(Aircraft& aircraft) {
        DefaultValues& defaults = aircraft.getDefaultValues();
        AssignedValues& assigned = aircraft.getAssignedValues();

        if (defaults.speed <= 0.0) {
            defaults.speed = DefaultValues().speed;
        }

        if (assigned.asdg_speed <= 0.0) {
            assigned.asdg_speed = defaults.speed;
        }
    }

    // --- Individual Handler Functions ---

    void handleTaxi(Aircraft& aircraft, const std::vector<std::string>& args) {
        if (!aircraft.onGround()) return;
        if (args.empty()) return;

        RouteManager& rm = aircraft.getRouteManager();
        rm.resetPath(aircraft.getAssignedValues(), aircraft.getDefaultValues());
        rm.resetHolding();
        rm.resetContext();

        // Reconstruct the route string from arguments
        std::string route_str;
        for (size_t i = 0; i < args.size(); ++i) {
            route_str += args[i] + (i == args.size() - 1 ? "" : " ");
        }

        // This part is a bit complex with "hs", we'll replicate the logic
        std::string taxi_part = route_str;
        std::string hold_part;
        size_t pos = route_str.find("hs ");
        if (pos != std::string::npos) {
            taxi_part = route_str.substr(0, pos);
            hold_part = route_str.substr(pos + 3);
        }

        // Process taxi route
        for (std::string s : split(taxi_part, " ")) {
            if (!s.empty()) {
                capitalize(s);
                rm.ground_route.push_back(trim(s));
            }
        }

        rm.prepareRoute(aircraft.getAirport(), aircraft.getState());
        if (rm.pollRoute()) {
            ensureTaxiSpeed(aircraft);
            aircraft.state = ACF_STATE::TAXING;
        }

        // Process holds
        if (!hold_part.empty()) {
            for (std::string s : split(hold_part, " ")) {
                if (!s.empty()) {
                    capitalize(s);
                    rm.HoldAt(aircraft.getAirport(), s);
                }
            }
        }
    }

    void handleHold(Aircraft& aircraft, const std::vector<std::string>& args) {
        if (aircraft.onGround() && aircraft.state == ACF_STATE::TAXING) {
            aircraft.state = ACF_STATE::HOLDING;
            AppendTextToConsole(s2ws(aircraft.getCallSign()) + L", holding.");
        }
    }

    void handleResume(Aircraft& aircraft, const std::vector<std::string>& args) {
        RouteManager& rm = aircraft.getRouteManager();
        if (aircraft.onGround() && aircraft.state == ACF_STATE::HOLDING) {
            if (!rm.HoldingFor) {
                if (rm.HoldingAt) {
                    rm.HoldingAt = nullptr;
                    if (!rm.HoldingDepart) aircraft.state = ACF_STATE::TAXING;
                }
                else if (!rm.lineup && !rm.queue_takeoff) {
                    aircraft.state = ACF_STATE::TAXING;
                }
            }
        }
    }

    void handleTurn(Aircraft& aircraft, const std::vector<std::string>& args, int turnDirection) {
        if (args.empty()) return;

        if (aircraft.onGround()) {
            aircraft.getRouteManager().resetPath(aircraft.getAssignedValues(), aircraft.getDefaultValues());
            aircraft.getRouteManager().resetHolding();
            aircraft.getRouteManager().resetContext();
        }

        aircraft.turnOri = turnDirection; // 0=left, 1=right, -1=auto
        aircraft.getAssignedValues().asgd_heading = hdg(atodd(args[0]));
        if (turnDirection == -1) { // Fly heading
            aircraft.getAssignedValues().asdg_roll = aircraft.getPerfValues().max_roll;
        }
    }

    void handleSpeed(Aircraft& aircraft, const std::vector<std::string>& args) {
        if (args.empty()) return;
        aircraft.turnOri = -1;
        if (!aircraft.getRouteManager().locked_rate) {
            aircraft.getDefaultValues().speed = aircraft.getAssignedValues().asdg_speed = atodd(args[0]);
        }
    }

    void handleHoldShort(Aircraft& aircraft, const std::vector<std::string>& args) {
        for (const auto& s_arg : args) {
            std::string s = s_arg;
            capitalize(s);
            aircraft.getRouteManager().HoldAt(aircraft.getAirport(), s);
        }
    }

    void handleSquawk(Aircraft& aircraft, const std::vector<std::string>& args) {
        if (args.empty()) return;
        const std::string& squawk = args[0];
        if (squawk.length() == 4 && is_digits(squawk)) {
            aircraft.setSquawkCode(squawk);
            updateSquawk(aircraft);
        }
    }

    void handleSquawkMode(Aircraft& aircraft, const std::vector<std::string>& args, int mode) {
        aircraft.setMode(mode);
        updateMode(aircraft);
    }

    void handleClearedForTakeoff(Aircraft& aircraft, const std::vector<std::string>& args) {
        RouteManager& rm = aircraft.getRouteManager();
        if (aircraft.onGround() && aircraft.state == ACF_STATE::HOLDING && rm.HoldingDepart) {
            if (rm.queue_takeoff && rm.lineup) {
                rm.lineup = false;
                rm.resetHolding();
                aircraft.state = ACF_STATE::TAXING;
            }
            else if (rm.runway_ctx && (rm.runway_ctx == rm.HoldingDepart) && rm.ground_cur) {
                Runway* runway = rm.runway_ctx;
                if (rm.prepareRunwayDeparturePath(runway)) {
                    ensureTaxiSpeed(aircraft);
                    rm.resetHolding();
                    rm.queue_takeoff = true;
                    rm.lineup = false;
                    aircraft.state = ACF_STATE::TAXING;
                }
            }
        }
    }

    void handleLineUpAndWait(Aircraft& aircraft, const std::vector<std::string>& args) {
        RouteManager& rm = aircraft.getRouteManager();
        if (aircraft.onGround() && aircraft.state == ACF_STATE::HOLDING && rm.HoldingDepart) {
            if (rm.runway_ctx && (rm.runway_ctx == rm.HoldingDepart) && rm.ground_cur) {
                Runway* runway = rm.runway_ctx;
                if (rm.prepareRunwayDeparturePath(runway)) {
                    ensureTaxiSpeed(aircraft);
                    rm.queue_takeoff = true;
                    rm.lineup = true;
                    aircraft.state = ACF_STATE::TAXING;
                }
            }
        }
    }

    void handleMessage(Aircraft& aircraft, const std::vector<std::string>& args) {
        if (args.empty()) return;
        std::string msg;
        for (size_t i = 0; i < args.size(); ++i) {
            msg += args[i] + (i == args.size() - 1 ? "" : " ");
        }
        sendUserMessage(aircraft, msg_freq, "", msg);
    }


    // --- Public Interface Implementation ---

    void initialize() {
        // Populate the map. Using lambdas to easily pass arguments.
        s_commandHandlers["taxi"] = [](Aircraft& ac, const auto& args) { handleTaxi(ac, args); };
        s_commandHandlers["hold"] = [](Aircraft& ac, const auto& args) { handleHold(ac, args); };
        s_commandHandlers["res"] = [](Aircraft& ac, const auto& args) { handleResume(ac, args); };
        s_commandHandlers["tl"] = [](Aircraft& ac, const auto& args) { handleTurn(ac, args, 0); }; // 0 for left
        s_commandHandlers["tr"] = [](Aircraft& ac, const auto& args) { handleTurn(ac, args, 1); }; // 1 for right
        s_commandHandlers["fh"] = [](Aircraft& ac, const auto& args) { handleTurn(ac, args, -1); }; // -1 for auto
        s_commandHandlers["spd"] = [](Aircraft& ac, const auto& args) { handleSpeed(ac, args); };
        s_commandHandlers["hs"] = [](Aircraft& ac, const auto& args) { handleHoldShort(ac, args); };
        s_commandHandlers["sq"] = [](Aircraft& ac, const auto& args) { handleSquawk(ac, args); };
        s_commandHandlers["sn"] = [](Aircraft& ac, const auto& args) { handleSquawkMode(ac, args, 1); }; // Mode C
        s_commandHandlers["ss"] = [](Aircraft& ac, const auto& args) { handleSquawkMode(ac, args, 0); }; // Standby
        s_commandHandlers["cto"] = [](Aircraft& ac, const auto& args) { handleClearedForTakeoff(ac, args); };
        s_commandHandlers["lw"] = [](Aircraft& ac, const auto& args) { handleLineUpAndWait(ac, args); };
        s_commandHandlers["ph"] = [](Aircraft& ac, const auto& args) { handleLineUpAndWait(ac, args); }; // Alias for lw
        s_commandHandlers["msg"] = [](Aircraft& ac, const auto& args) { handleMessage(ac, args); };

        printf("Command handlers initialized.\n");
    }

    void processCommand(Aircraft& aircraft, const std::string& commandString) {
        if (commandString.empty()) {
            return;
        }

        std::vector<std::string> parts = split(trim(commandString), " ");
        if (parts.empty()) {
            return;
        }

        std::string commandKey = parts[0];
        std::transform(commandKey.begin(), commandKey.end(), commandKey.begin(), ::tolower);

        auto it = s_commandHandlers.find(commandKey);
        if (it != s_commandHandlers.end()) {
            // Found the handler, execute it.
            // Create a vector of arguments (all parts after the command key).
            std::vector<std::string> args(parts.begin() + 1, parts.end());
            it->second(aircraft, args);
        }
        else {
            // Optional: Log unhandled command
            // AppendTextToConsole(L"Unknown command: " + s2ws(commandKey));
        }
    }

} // namespace CommandHandlers
