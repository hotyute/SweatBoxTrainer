#include "usermanager.h"

#include <any>

#include "SweatBoxTrainer.h"
#include "tools.h"
#include "packets_out.h"
#include "globals.h"

std::vector<Aircraft*> userStorage1;

void processIncomingPackets(Aircraft* aircraft, int opCode, BasicStream& stream) {
	if (opCode == 7) {
		char msg[256];
		stream.readString(msg);
	}

	if (opCode == 8) {
		char wx[256];
		stream.readString(wx);
	}

	if (opCode == 9) {
		//create new user packet
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
	if (opCode == 10)
	{
		//update Cycle Change
		long long time = stream.readQWord();
		printf("time_change: %s, %lld\n", aircraft->getCallSign().c_str(), time);
		aircraft->setUpdateTime(time);
	}
	if (opCode == 11) {// recieve message
		char callsign[25];
		stream.readString(callsign);
		char msg[2048];
		stream.readString(msg);
	}
	if (opCode == 12) {//delete user packet
		int index = stream.read_unsigned_short();
	}
	if (opCode == 13) {
		//ping packet
	}
	if (opCode == 14)
	{
		//Pilot  Update Packet
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
	if (opCode == 15)
	{
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
				std::size_t pos = message.find(aircraft->getCallSign() + ", ");
				if (pos != std::string::npos)
				{
					processCommands(*aircraft, message.substr(pos + pre_cursor.length()));
				}
			}
		}
		//handle frequency commands
	}
	if (opCode == 16)
	{
		int index = stream.read_unsigned_short();
		int i = stream.read_unsigned_byte();
		int mode = i << 4, heavy = i & 0xF;;
	}
	if (opCode == 17)
	{//update flight plan packet
		int index = stream.read_unsigned_short();
		int cur_cycle = stream.read_unsigned_short();
		AV_CLIENT type = static_cast<AV_CLIENT>(stream.read_unsigned_byte());

		if (type == AV_CLIENT::PILOT) {
			int flightRules = stream.read_unsigned_byte();
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
		}
	}
	if (opCode == 18)
	{
		//Controller Update Packet
		int index = stream.read_unsigned_short();
		long long lat = stream.readQWord();
		long long lon = stream.readQWord();
		double latitude = *reinterpret_cast<double*>(&lat);
		double longitude = *reinterpret_cast<double*>(&lon);
		int flags = stream.read_unsigned_byte();
	}
	if (opCode == 19)
	{
		int index = stream.read_unsigned_short();
		int vis_range = stream.read_unsigned_short();
	}
	if (opCode == 20)
	{
		int index = stream.read_unsigned_short();
		char code[20];
		stream.readString(code);
	}
	if (opCode == 21)
	{
		int index = stream.read_unsigned_short();
		int flags = stream.read_unsigned_byte();
		int freq = stream.read_unsigned_int();
	}
	if (opCode == 22)
	{
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
	if (opCode == 23)
	{
		int index = stream.read_unsigned_short();
		int script_idx = stream.read_unsigned_short();
	}
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
}
