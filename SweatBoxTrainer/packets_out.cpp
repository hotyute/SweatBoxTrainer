#include "packets_out.h"

#include <any>

#include "basic_stream.h"
#include <stdarg.h>
#include <stdio.h>

void sendPositionUpdates(Aircraft& user) {
	BasicStream out = BasicStream(40);
	out.create_frame_var_size(_AIRCRAFT_POS_UPDATE);
	double lat = user.getLatitude();
	double lon = user.getLongitude();
	const long long latitude = *reinterpret_cast<long long*>(&lat);
	const long long longitude = *reinterpret_cast<long long*>(&lon);
	out.write_qword(latitude);
	out.write_qword(longitude);
	const long long infoHash = ((static_cast<long long>(static_cast<int>((user.getPitch() * 1024.0) / -360.0))) << 22)
		+ ((static_cast<long long>(static_cast<int>((user.getRoll() * 1024.0) / -360.0))) << 12)
		+ ((static_cast<long long>(static_cast<int>((user.getHeading() * 1024.0) / 360.0))) << 2);
	out.write_qword(infoHash);
	out.write_short(static_cast<int>(user.getSpeed()));
	out.write_qword(static_cast<long long>(user.getAltitude()));
	out.end_frame_var_size();
	user.getConnection().sendMessage(&out);
}

void sendFlightPlan(Aircraft& user) {
	BasicStream out = BasicStream(256);
	FlightPlan& fp = user.getFlightPlan();
	out.create_frame_var_size_word(_SEND_FLIGHT_PLAN);
	out.write_short(fp.cycle);
	out.write_short(user.getUserIndex());
	out.write_byte(fp.flightRules);
	out.write_string(fp.squawkCode.c_str());
	out.write_string(fp.departure.c_str());
	out.write_string(fp.arrival.c_str());
	out.write_string(fp.alternate.c_str());
	out.write_string(fp.cruise.c_str());
	out.write_string(fp.acType.c_str());
	out.write_string(fp.scratchPad.c_str());
	out.write_string(fp.route.c_str());
	out.write_string(fp.remarks.c_str());
	out.end_frame_var_size_word();
	user.getConnection().sendMessage(&out);
}

void updateMode(Aircraft& user) {
	BasicStream out = BasicStream(2);
	out.create_frame(_UPDATE_MODE);
	out.write_byte(user.getMode());
	user.getConnection().sendMessage(&out);
}

void updateSquawk(Aircraft& user) {
	BasicStream out = BasicStream(15);
	out.create_frame_var_size(_UPDATE_TRANSPONDER);
	out.write_string(user.getSquawkCode().c_str());
	out.end_frame_var_size();
	user.getConnection().sendMessage(&out);
}

void sendDisconnect(Aircraft& user) {
	BasicStream out = BasicStream(2);
	out.create_frame(_DISCONNECT_PACKET);
	out.write_byte(0);
	user.getConnection().sendMessage(&out);
}

void sendPingPacket(Aircraft& user) {
	BasicStream out = BasicStream(2);
	out.create_frame(_PING);
	user.getConnection().sendMessage(&out);
}

void sendUserMessage(Aircraft& user, int frequency, std::string to, std::string message) {
	BasicStream out = BasicStream(512);
	out.create_frame_var_size_word(_USER_MESSAGE);
	out.write_string(to.c_str());
	out.write_3byte(frequency);//99998 = 199.998
	out.write_string(message.c_str());
	out.end_frame_var_size_word();
	user.getConnection().sendMessage(&out);
}

void sendPrimFreq(Aircraft& user) {
	BasicStream out = BasicStream(8);
	out.create_frame(_PRIMARY_FREQ);
	out.write_byte(0);
	out.write_3byte(user.frequency[0]);
	out.write_3byte(user.frequency[1]);
	user.getConnection().sendMessage(&out);
}

void sendTempData(Aircraft& user, std::string &assembly, const void* &data, ...) {
	BasicStream out = BasicStream(256);
	out.create_frame_var_size_word(_TEMP_DATA);
	out.write_string(assembly.c_str());
	va_list args;
	va_start(args, data);
	int header = va_arg(args, int);
	for (int i_11_ = assembly.length() - 1; i_11_ >= 0; i_11_--) {
		if (assembly.at(i_11_) == 's')
			out.write_string(va_arg(args, std::string).c_str());
		else if (assembly.at(i_11_) == 'l')
			out.write_qword(va_arg(args, long long));
		else
			out.write_int(va_arg(args, int));
	};
	va_end(args);
	out.write_int(header);
	out.end_frame_var_size_word();
	user.getConnection().sendMessage(&out);
}
