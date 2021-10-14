#include "usermanager.h"

#include "SweatBoxTrainer.h"
#include "tools.h"
#include "packets_out.h"

std::vector<Aircraft*> userStorage1;

void processIncomingPackets(Aircraft* aircraft, int opCode, Stream& stream) {
	if (opCode == 9) {
		//create new user packet
		int index = stream.readUnsignedWord();
		AV_CLIENT type = static_cast<AV_CLIENT>(stream.readUnsignedByte());
		char callSign1[1024], full_name[1024], username[1024];
		stream.readString(callSign1);
		stream.readString(username);
		stream.readString(full_name);
		int vis_range = stream.readUnsignedWord();
		long long lat = stream.readQWord();
		long long lon = stream.readQWord();
		if (type == AV_CLIENT::CONTROLLER) 
		{
			//do nothing but read the stream
			int controller_rating = stream.readUnsignedByte();
			int controller_position = stream.readUnsignedByte();

		}
		else if (type == AV_CLIENT::PILOT) 
		{
			char acfTitle[1024];
			stream.readString(acfTitle);
			char trans_code[1024];
			stream.readString(trans_code);
			int squawkMode = stream.readUnsignedByte();
			long long hash = stream.readQWord();


			//user1->setUserIndex(index);
			//userStorage1[index] = user1;
		}

	}
	if (opCode == 10) 
	{
		//update Cycle Change
		long long time = stream.readQWord();
		printf("time_change: %s, %lld\n", aircraft->getIdentity()->callsign.c_str(), time);
		aircraft->setUpdateTime(time);
	}
	if (opCode == 11) {// recieve message
		char callsign[25];
		stream.readString(callsign);
		char msg[2048];
		stream.readString(msg);
	}
	if (opCode == 12) {//delete user packet
		int index = stream.readUnsignedWord();
	}
	if (opCode == 13) {
		//ping packet
	}
	if (opCode == 14) 
	{
		//Pilot  Update Packet
		int index = stream.readUnsignedWord();
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
		int groundSpeed = stream.readUnsignedWord();
		long long alt = stream.readQWord();
		double altitude = *(double*)&alt;
	}
	if (opCode == 15) 
	{
		int index = stream.readUnsignedWord();
		int frequency = stream.readDWord();
		char msg[2048];
		stream.readString(msg);
		//handle frequency commands
	}
	if (opCode == 16) 
	{
		int index = stream.readUnsignedWord();
		int mode = stream.readUnsignedByte();
	}
	if (opCode == 17) 
	{//update flight plan packet
		int index = stream.readUnsignedWord();
		int cur_cycle = stream.readUnsignedWord();
		AV_CLIENT type = static_cast<AV_CLIENT>(stream.readUnsignedByte());

		if (type == AV_CLIENT::PILOT) {
			int flightRules = stream.readUnsignedByte();
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
		int index = stream.readUnsignedWord();
		long long lat = stream.readQWord();
		long long lon = stream.readQWord();
		double latitude = *(double*)&lat;
		double longitude = *(double*)&lon;
		int flags = stream.readUnsignedByte();
	}
	if (opCode == 19)
	{
		int index = stream.readUnsignedWord();
		int vis_range = stream.readUnsignedWord();
	}
	if (opCode == 20)
	{
		int index = stream.readUnsignedWord();
		char code[20];
		stream.readString(code);
	}
}

Aircraft* createAircraft(std::string callsign, double latitude, double longitude, double heading, double speed, int altitude, 
	int verticalSpeed, int mode, std::string squawkCode) {
	Aircraft* cur = new Aircraft();
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

	AcfMap[cur->getIdentity()->callsign] = cur;

	addUserToLB(cur);
	return cur;
}

void disconnect(Aircraft* aircraft, bool queue)
{
	sendDisconnect(*aircraft);
	aircraft->getConnection().queue_clean = queue;
	aircraft->getConnection().disconnect_socket();
	aircraft->position_updates->toggle_pause();
	aircraft->connected = false;
}
