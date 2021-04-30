#include "usermanager.h"

#include "SweatBoxTrainer.h"
#include "tools.h"

std::vector<Aircraft*> userStorage1;
std::unordered_map<std::string, Aircraft*> users_map;

void decodePackets(Aircraft* aircraft, int opCode, Stream& stream) {
	if (opCode == 10) {
		//update Cycle Change
		aircraft->setUpdateTime(stream.readQWord());
	}
	if (opCode == 13) {
		//ping packet
	}
	if (opCode == 9) {
		//create new user packet
		int index = stream.readUnsignedWord();
		int type = stream.readUnsignedByte();
		char callSign1[1024], full_name[1024], username[1024];
		stream.readString(callSign1);
		stream.readString(username);
		stream.readString(full_name);

		if (type == CONTROLLER_CLIENT) {

		}
		else if (type == PILOT_CLIENT) {
			char acfTitle[1024];
			stream.readString(acfTitle);
			char trans_code[1024];
			stream.readString(trans_code);
			int squawkMode = stream.readUnsignedByte();


			//user1->setUserIndex(index);
			//userStorage1[index] = user1;
		}

	}
	if (opCode == 12) {//delete user packet
		int index = stream.readUnsignedWord();
	}
	if (opCode == 14) {
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
	if (opCode == 16) {
		int index = stream.readUnsignedWord();
		int mode = stream.readUnsignedByte();
	}
	if (opCode == 11) {// recieve message
		int index = stream.readUnsignedWord();
		char msg[2048];
		stream.readString(msg);
	}
	if (opCode == 17) {//update flight plan packet
		int index = stream.readUnsignedWord();
		int cur_cycle = stream.readUnsignedWord();
		int type = stream.readUnsignedByte();

		if (type == PILOT_CLIENT) {
			int flightRules = stream.readUnsignedByte();
			char assigned_squawk[5], departure[5], arrival[5], alternate[5], cruise[6], ac_type[8], scratch[5], route[128], remarks[128];
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
}
