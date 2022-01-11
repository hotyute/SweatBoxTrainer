#include "packets_out.h"

#include <iostream>

void sendPositionUpdates(Aircraft& user) {
	Stream& out = Stream(40);
	out.createFrameVarSize(_AIRCRAFT_POS_UPDATE);
	double lat = user.getLatitude();
	double lon = user.getLongitude();
	long long latitude = *(long long*)&lat;
	long long longitude = *(long long*)&lon;
	out.writeQWord(latitude);
	out.writeQWord(longitude);
	long long infoHash = ((static_cast<long long>((int)((user.getPitch() * 1024.0) / -360.0))) << 22)
		+ ((static_cast<long long>((int)((user.getRoll() * 1024.0) / -360.0))) << 12)
		+ ((static_cast<long long>((int)((user.getHeading() * 1024.0) / 360.0))) << 2);
	out.writeQWord(infoHash);
	out.writeWord((int)user.getSpeed());
	out.writeQWord((long long)user.getAltitude());
	out.endFrameVarSize();
	user.getConnection().sendMessage(&out);
}

void sendFlightPlan(Aircraft& user) {
	Stream& out = Stream(256);
	FlightPlan& fp = user.getFlightPlan();
	out.createFrameVarSizeWord(_SEND_FLIGHT_PLAN);
	out.writeWord(fp.cycle);
	out.writeWord(user.getUserIndex());
	out.writeByte(fp.flightRules);
	out.writeString((char*)fp.squawkCode.c_str());
	out.writeString((char*)fp.departure.c_str());
	out.writeString((char*)fp.arrival.c_str());
	out.writeString((char*)fp.alternate.c_str());
	out.writeString((char*)fp.cruise.c_str());
	out.writeString((char*)fp.acType.c_str());
	out.writeString((char*)fp.scratchPad.c_str());
	out.writeString((char*)fp.route.c_str());
	out.writeString((char*)fp.remarks.c_str());
	out.endFrameVarSizeWord();
	user.getConnection().sendMessage(&out);
}

void updateMode(Aircraft& user) {
	Stream& out = Stream(2);
	out.createFrame(_UPDATE_MODE);
	out.writeByte(user.getMode());
	user.getConnection().sendMessage(&out);
}

void updateSquawk(Aircraft& user) {
	Stream& out = Stream(15);
	out.createFrameVarSize(_UPDATE_TRANSPONDER);
	out.writeString((char*)user.getSquawkCode().c_str());
	out.endFrameVarSize();
	user.getConnection().sendMessage(&out);
}

void sendDisconnect(Aircraft& user) {
	Stream& out = Stream(2);
	out.createFrame(_DISCONNECT_PACKET);
	out.writeByte(0);
	user.getConnection().sendMessage(&out);
}

void sendPingPacket(Aircraft& user) {
	Stream& out = Stream(2);
	out.createFrame(_PING);
	user.getConnection().sendMessage(&out);
}

void sendUserMessage(Aircraft& user, int frequency, std::string to, std::string message) {
	Stream& out = Stream(512);
	out.createFrameVarSizeWord(_USER_MESSAGE);
	out.writeString((char*)to.c_str());
	out.write3Byte(frequency);//99998 = 199.998
	out.writeString((char*)message.c_str());
	out.endFrameVarSizeWord();
	user.getConnection().sendMessage(&out);
}

void sendPrimFreq(Aircraft& user) {
	Stream& out = Stream(8);
	out.createFrame(_PRIMARY_FREQ);
	out.writeByte(0);
	out.write3Byte(user.frequency[0]);
	out.write3Byte(user.frequency[1]);
	user.getConnection().sendMessage(&out);
}
