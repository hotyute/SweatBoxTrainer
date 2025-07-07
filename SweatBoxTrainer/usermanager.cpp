#include "usermanager.h"

#include <any>
#include <functional> // Required for std::function
#include <unordered_map> // Required for the dispatch table

#include "SweatBoxTrainer.h"
#include "tools.h"
#include "packets_out.h"
#include "globals.h"

std::vector<Aircraft*> userStorage1;

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

void handleUpdateCycleChange(Aircraft* aircraft, BasicStream& stream) {
	long long time = stream.readQWord();
	printf("time_change: %s, %lld\n", aircraft->getCallSign().c_str(), time);
	aircraft->setUpdateTime(time);
}

void handleUserMessage(Aircraft* aircraft, BasicStream& stream) {
	char callsign[25];
	stream.readString(callsign);
	char msg[2048];
	stream.readString(msg);
	// You could append this to the console, process it, etc.
	// e.g., AppendTextToConsole(s2ws(std::string(callsign) + ": " + msg));
}

void handlePilotUpdate(Aircraft* aircraft, BasicStream& stream) {
	int index = stream.read_unsigned_short();
	long long lat = stream.readQWord();
	long long lon = stream.readQWord();
	double latitude = *(double*)&lat;
	double longitude = *(double*)&lon;
	long long hash = stream.readQWord();
	unsigned long long num2 = hash >> 22;
	unsigned int num3 = hash >> 12 & 1023u;
	unsigned int num4 = hash >> 2 & 1023u;
	double pitch = num2 / 1024.0 * -360.0;
	double roll = num3 / 1024.0 * -360.0;
	double heading = num4 / 1024.0 * 360.0;
	int groundSpeed = stream.read_unsigned_short();
	long long alt = stream.readQWord();
	double altitude = *(double*)&alt;
}

void handleFrequencyCommands(Aircraft* aircraft, BasicStream& stream) {
	int index = stream.read_unsigned_short();
	int frequency = stream.read3Byte();
	bool asel = stream.read_unsigned_byte() == 1;
	char msg[2048];
	stream.readString(msg);
	if (frequency == command_freq)
	{
		std::string message = std::string(msg);
		if (asel)
		{
			std::string pre_cursor = aircraft->getCallSign() + ", ";
			std::size_t pos = message.find(pre_cursor);
			if (pos != std::string::npos)
			{
				processCommands(*aircraft, message.substr(pos + pre_cursor.length()));
			}
		}
	}
}

void handleFlightPlanUpdate(Aircraft* aircraft, BasicStream& stream) {
	int index = stream.read_unsigned_short();
	int cur_cycle = stream.read_unsigned_short();
	AV_CLIENT type = static_cast<AV_CLIENT>(stream.read_unsigned_byte());

	// Again, find aircraft by index if this can be for other planes.
	// If it's always for 'aircraft', then we can proceed.
	if (type == AV_CLIENT::PILOT) {
		FlightPlan& fp = aircraft->getFlightPlan();
		char assigned_squawk[5], departure[5], arrival[5], alternate[5], cruise[6], ac_type[9], scratch[5], route[128], remarks[128];
		stream.readString(assigned_squawk);
		stream.readString(departure);
		stream.readString(arrival);
		stream.readString(alternate);
		stream.readString(cruise);
		stream.readString(ac_type);
		stream.readString(scratch);
		stream.readString(route);
		stream.readString(remarks);
		// Now assign these values to fp
		fp.squawkCode = assigned_squawk;
	}
}

void handleCreateUser(Aircraft* aircraft, BasicStream& stream) {
	int index = stream.read_unsigned_short();
	AV_CLIENT type = static_cast<AV_CLIENT>(stream.read_unsigned_byte());
	char callSign1[1024], full_name[1024], username[1024];
	stream.readString(callSign1);
	stream.readString(username);
	stream.readString(full_name);
	int vis_range = stream.read_unsigned_short();
	long long lat = stream.readQWord();
	long long lon = stream.readQWord();
	if (type == AV_CLIENT::CONTROLLER)
	{
		//do nothing but read the stream
		int controller_rating = stream.read_unsigned_byte();
		int controller_position = stream.read_unsigned_byte();
		int controller_freq = stream.read3Byte();
	}
	else if (type == AV_CLIENT::PILOT)
	{
		char acfTitle[1024];
		stream.readString(acfTitle);
		char trans_code[1024];
		stream.readString(trans_code);
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
		//user1->setUserIndex(index);
		//userStorage1[index] = user1;
	}
}

void handleControllerUpdate(Aircraft* aircraft, BasicStream& stream) {
	//Controller Update Packet
	int index = stream.read_unsigned_short();
	long long lat = stream.readQWord();
	long long lon = stream.readQWord();
	double latitude = *reinterpret_cast<double*>(&lat);
	double longitude = *reinterpret_cast<double*>(&lon);
	int flags = stream.read_unsigned_byte();
}

void handleScript(Aircraft* aircraft, BasicStream& stream) {
	int index = stream.read_unsigned_short();
	int script_idx = stream.read_unsigned_short();
	std::string assembly = stream.read_string();
	std::vector<std::any> objects(assembly.length() + 1);
	for (int i_11_ = assembly.length() - 1; i_11_ >= 0; i_11_--)
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
	packetHandlers[12] = [](Aircraft* ac, BasicStream& s) { int index = s.read_unsigned_short(); /* Logic to delete user */ };
	packetHandlers[13] = [](Aircraft* ac, BasicStream& s) { /* Ping packet, do nothing or reply */ };
	packetHandlers[16] = [](Aircraft* ac, BasicStream& s) { int index = s.read_unsigned_short(); int i = s.read_unsigned_byte(); int mode = i << 4, heavy = i & 0xF; };
	packetHandlers[19] = [](Aircraft* ac, BasicStream& s) { int index = s.read_unsigned_short(); int vis_range = s.read_unsigned_short(); };
	packetHandlers[20] = [](Aircraft* ac, BasicStream& s) { int index = s.read_unsigned_short(); char code[20]; s.readString(code); };
	packetHandlers[21] = [](Aircraft* ac, BasicStream& s) { int index = s.read_unsigned_short(); int flags = s.read_unsigned_byte(); int freq = s.read_unsigned_int(); };
	packetHandlers[23] = [](Aircraft* ac, BasicStream& s) { int index = s.read_unsigned_short(); int script_idx = s.read_unsigned_short(); };

	printf("Packet handlers initialized.\n");
}

Aircraft* createAircraft(std::string callsign, double latitude, double longitude, double heading, double speed, double altitude,
	double verticalSpeed, int mode, std::string squawkCode) {
	auto cur = std::make_unique<Aircraft>();
	cur->frequency[0] = command_freq;
	cur->frequency[1] = msg_freq;
	AssignedValues& av = cur->getAssignedValues();
	cur->lock();
	cur->setType(AV_CLIENT::PILOT);
	cur->setHeavy(false);
	cur->getIdentity()->callsign = callsign;
	cur->getIdentity()->username = "971202";
	cur->getIdentity()->login_name = "Samuel Mason";
	cur->getIdentity()->password = "password";
	cur->getIdentity()->pilot_rating = 1;
	cur->setLatitude(latitude);
	cur->setLongitude(longitude);
	cur->setSpeed(av.asdg_speed = speed);
	cur->setHeading(av.asgd_heading = heading);
	cur->setAltitude(av.asdg_altitude = altitude);
	cur->setVerticalSpeed(verticalSpeed);
	cur->setMode(mode);
	cur->unlock();

	cur->setSquawkCode(squawkCode);

	cur->getConnection().init_set();

	Aircraft* raw_ptr = cur.get();

	{
		std::lock_guard<std::mutex> lock(g_acfMapMutex);
		AcfMap[callsign] = std::move(cur); // Move ownership into the map
	}

	addUserToLB(raw_ptr);
	return raw_ptr;
}

void disconnect(Aircraft* aircraft, bool queue)
{
	closesocket(aircraft->getConnection().sConnect);
	sendDisconnect(*aircraft);
	aircraft->getConnection().disconnect_socket();
	aircraft->connected = false;

	// TODO: Safely remove the aircraft from AcfMap
	// {
	//     std::lock_guard<std::mutex> lock(g_acfMapMutex);
	//     AcfMap.erase(aircraft->getCallSign());
	// }
}
