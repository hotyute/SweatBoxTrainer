#include "packets_in.h"

#include <any>
#include <functional> // Required for std::function
#include <unordered_map> // Required for the dispatch table

#include "tools.h"
#include "globals.h"
#include "aircraft/command_handler.h"
#include "sim/simulation_context.h"

// =======================================================================
// 1. Define the Packet Handler Dispatch Table
// =======================================================================
// This map will store our packet handlers. The key is the integer opCode,
// and the value is a function that takes Aircraft* and BasicStream&.
static std::unordered_map<int, std::function<void(Aircraft*, BasicStream&)>> packetHandlers;


// =======================================================================
// 2. Create Individual Handler Functions
// =======================================================================
// We take the logic from each 'if' block and move it into its own function.

static void handleUpdateCycleChange(Aircraft* connection_owner, BasicStream& stream) {
	long long time = stream.readQWord();
	printf("time_change: %s, %lld\n", connection_owner->getCallSign().c_str(), time);
	connection_owner->setUpdateTime(time);
	if (g_app.threadPool) { // Safety check
		connection_owner->startPositionUpdates(*g_app.threadPool);
	}
}

static void handleUserMessage(Aircraft* connection_owner, BasicStream& stream) {
	std::string callsign = stream.read_std_string();
	std::string msg = stream.read_std_string();
	// You could append this to the console, process it, etc.
	// e.g., AppendTextToConsole(s2ws(std::string(callsign) + ": " + msg));
}

static void handlePilotUpdate(Aircraft* connection_owner, BasicStream& stream) {
	int index = stream.read_unsigned_short();

	auto& ctx = SimulationContext::instance();
	// The calling SocketPollingTask holds the mutex, so this is safe.
	Aircraft* target_aircraft = ctx.findAircraftByIndex(index);
	if (!target_aircraft) return; // Received update for an unknown aircraft
	

	double latitude = std::bit_cast<double>(stream.readQWord());
	double longitude = std::bit_cast<double>(stream.readQWord());
	long long hash = stream.readQWord();
	unsigned long long num2 = hash >> 22;
	unsigned int num3 = hash >> 12 & 1023u;
	unsigned int num4 = hash >> 2 & 1023u;
	double pitch = num2 / 1024.0 * -360.0;
	double roll = num3 / 1024.0 * -360.0;
	double heading = num4 / 1024.0 * 360.0;
	int groundSpeed = stream.read_unsigned_short();
	double altitude = std::bit_cast<double>(stream.readQWord());

	// Note: Here you would update the state of 'target_aircraft', not 'connection_owner'
	// For now, this handler just reads the data.
}

static void handleFrequencyCommands(Aircraft* connection_owner, BasicStream& stream) {
	int index = stream.read_unsigned_short();
	int frequency = stream.read3Byte();
	bool asel = stream.read_unsigned_byte() == 1;
	std::string msg = stream.read_std_string();
	if (frequency == command_freq)
	{
		std::string message = std::string(msg);
		if (asel)
		{
			std::string pre_cursor = connection_owner->getCallSign() + ", ";
			std::size_t pos = message.find(pre_cursor);
			if (pos != std::string::npos)
			{
				CommandHandlers::processCommand(*connection_owner, message.substr(pos + pre_cursor.length()));
			}
		}
	}
}

static void handleFlightPlanUpdate(Aircraft* connection_owner, BasicStream& stream) {
	int index = stream.read_unsigned_short();

	auto& ctx = SimulationContext::instance();
	Aircraft* target_aircraft = ctx.findAircraftByIndex(index);
	if (!target_aircraft) return;

	int cur_cycle = stream.read_unsigned_short();
	AV_CLIENT type = static_cast<AV_CLIENT>(stream.read_unsigned_byte());

	if (type == AV_CLIENT::PILOT) {
		FlightPlan& fp = target_aircraft->getFlightPlan();
		stream.read_unsigned_byte();
		std::vector<std::string> vars(9);
		for (auto& var : vars) { var = stream.read_std_string(); }
		// Now assign these values to fp
		fp.squawkCode = vars[0];
		fp.departure = vars[1];
		fp.arrival = vars[2];
		//... and so on
	}
}

static void handleCreateUser(Aircraft* connection_owner, BasicStream& stream) {
	int index = stream.read_unsigned_short();
	AV_CLIENT type = static_cast<AV_CLIENT>(stream.read_unsigned_byte());
	std::string callSign1 = stream.read_std_string();
	std::string username = stream.read_std_string();
	std::string full_name = stream.read_std_string();
	int vis_range = stream.read_unsigned_short();
	double latitude = std::bit_cast<double>(stream.readQWord());
	double longitude = std::bit_cast<double>(stream.readQWord());

	if (type == AV_CLIENT::CONTROLLER)
	{
		//do nothing but read the stream
		int controller_rating = stream.read_unsigned_byte();
		int controller_position = stream.read_unsigned_byte();
		int controller_freq = stream.read3Byte();
	}
	else if (type == AV_CLIENT::PILOT)
	{
		std::string acfTitle = stream.read_std_string();
		std::string trans_code = stream.read_std_string();
		int m = stream.read_unsigned_byte();
		long long hash = stream.readQWord();
		int squawkMode = m >> 4;
		bool heavy = (m & 0xf) == 1;
		unsigned long long num2 = hash >> 22;
		unsigned int num3 = hash >> 12 & 1023u;
		unsigned int num4 = hash >> 2 & 1023u;
		double pitch = num2 / 1024.0 * -360.0;
		double roll = num3 / 1024.0 * -360.0;
		double heading = num4 / 1024.0 * 360.0;

		// The calling SocketPollingTask holds the mutex, so this is safe.
		auto new_aircraft = createAircraft(callSign1, latitude, longitude, heading, 0, 0, 0, squawkMode, trans_code);
		if (new_aircraft) {
			new_aircraft->setUserIndex(index);
			auto& ctx = SimulationContext::instance();
			ctx.indexToCallsignMap()[index] = new_aircraft->getCallSign();
			// Further initialization for the new aircraft...
			new_aircraft->setHeavy(heavy);
			new_aircraft->setAcfTitle(acfTitle);
			ctx.aircraft()[new_aircraft->getCallSign()] = std::move(new_aircraft); // Caller adds to context
		}
	}
}

static void handleControllerUpdate(Aircraft* connection_owner, BasicStream& stream) {
	int index = stream.read_unsigned_short();
	// As with pilot update, you'd find the controller and update their state.
	// For now, just reading the data.
	double latitude = std::bit_cast<double>(stream.readQWord());
	double longitude = std::bit_cast<double>(stream.readQWord());
	int flags = stream.read_unsigned_byte();
}

static void handleScript(Aircraft* connection_owner, BasicStream& stream) {
	int index = stream.read_unsigned_short();
	int script_idx = stream.read_unsigned_short();
	std::string assembly = stream.read_string();
	std::vector<std::any> objects(assembly.length() + 1);
	for (int i_11_ = static_cast<int>(assembly.length()) - 1; i_11_ >= 0; i_11_--)
	{
		if (assembly.at(i_11_) == 's')
			objects[i_11_ + 1] = stream.read_string();
		else if (assembly.at(i_11_) == 'l')
			objects[i_11_ + 1] = stream.readQWord();
		else
			objects[i_11_ + 1] = (int)stream.read_unsigned_int();
	}
	objects[0] = (int)stream.read_unsigned_int();
}

// =======================================================================
// 3. The New processIncomingPackets Implementation
// =======================================================================
void processIncomingPackets(Aircraft* aircraft, int opCode, BasicStream& stream) {
	auto it = packetHandlers.find(opCode);
	if (it != packetHandlers.end()) {
		// Found a handler, execute it
		it->second(aircraft, stream);
	}
	else {
		// No handler found for this opCode
		printf("Unhandled packet received. OpCode: %d\n", opCode);
		// You might want to skip the stream content based on packet length
		// if you have that information, to prevent desync.
	}
}

// =======================================================================
// 4. Initialize the Dispatch Table
// =======================================================================
void initializePacketHandlers() {
	// --- Register all your packet handlers here ---

	// Using named functions for complex packets
	packetHandlers[9] = handleCreateUser;
	packetHandlers[10] = handleUpdateCycleChange;
	packetHandlers[11] = handleUserMessage;
	packetHandlers[14] = handlePilotUpdate;
	packetHandlers[15] = handleFrequencyCommands;
	packetHandlers[17] = handleFlightPlanUpdate;
	packetHandlers[18] = handleControllerUpdate;
	packetHandlers[22] = handleScript;

	// Using lambdas for simple or placeholder handlers
	packetHandlers[7] = [](Aircraft* ac, BasicStream& s) { char msg[256]; s.readString(msg); };
	packetHandlers[8] = [](Aircraft* ac, BasicStream& s) { char wx[256]; s.readString(wx); };
	packetHandlers[12] = [](Aircraft* connection_owner, BasicStream& s) {
		int index = s.read_unsigned_short();
		auto& ctx = SimulationContext::instance();
		Aircraft* target_aircraft = ctx.findAircraftByIndex(index);
		if (target_aircraft) {
			// Logic to delete user, e.g., mark for removal
		}
		};
	packetHandlers[13] = [](Aircraft* ac, BasicStream& s) { /* Ping packet, do nothing or reply */ };
	packetHandlers[16] = [](Aircraft* connection_owner, BasicStream& s) {
		int index = s.read_unsigned_short();
		int i = s.read_unsigned_byte();
		int mode = i << 4;
		bool heavy = (i & 0xf) == 1;
		auto& ctx = SimulationContext::instance();
		Aircraft* target_aircraft = ctx.findAircraftByIndex(index);
		if (target_aircraft) {
			target_aircraft->setMode(mode);
			target_aircraft->setHeavy(heavy);
		}
		};
	packetHandlers[19] = [](Aircraft* connection_owner, BasicStream& s) {
		int index = s.read_unsigned_short();
		int vis_range = s.read_unsigned_short();
		// find aircraft and set visibility
		};
	packetHandlers[20] = [](Aircraft* connection_owner, BasicStream& s) {
		int index = s.read_unsigned_short();
		char code[20];
		s.readString(code);
		// find aircraft and set something with code
		};
	packetHandlers[21] = [](Aircraft* connection_owner, BasicStream& s) {
		int index = s.read_unsigned_short();
		int flags = s.read_unsigned_byte();
		int freq = s.read_unsigned_int();
		// find aircraft and set something with flags/freq
		};
	packetHandlers[23] = [](Aircraft* connection_owner, BasicStream& s) {
		int index = s.read_unsigned_short();
		int script_idx = s.read_unsigned_short();
		// find aircraft and do something with script
		};

	printf("Packet handlers initialized.\n");
}