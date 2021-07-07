#include "packets_out.h"

#include <iostream>

void sendPositionUpdates(Aircraft& user) {
	Stream& out = Stream(40);
	out.createFrameVarSize(_AIRCRAFT_POS_UPDATE);
	out.writeWord(user.getUserIndex());
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
	out.writeQWord(user.getAltitude());
	out.endFrameVarSize();
	user.getConnection().sendMessage(&out);
}

void sendFlightPlan(Aircraft& user) {
	Stream& out = Stream(256);
	FlightPlan& fp = user.getFlightPlan();
	out.createFrameVarSizeWord(_SEND_FLIGHT_PLAN);
	out.writeWord(fp.cycle);
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
