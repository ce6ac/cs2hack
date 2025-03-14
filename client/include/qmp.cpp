#include "qmp.h"
 
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

qemu::qmp::qmp()
		: connected_(false),
			socket_(0) {}
 
bool qemu::qmp::setup(std::string_view address, uint32_t port) {
	if (connected_) {
		printf("qmp: connection is already open");
		return true;
	}
 
	socket_ = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_ == -1) {
		printf("qmp: could not create socket descriptor");
		return false;
	}
 
	struct sockaddr_in socket_address{};
	socket_address.sin_family = AF_INET;
	socket_address.sin_port = htons(port);
	socket_address.sin_addr.s_addr = inet_addr(address.data());
 
	// Connect to the socket.
	auto result = connect(
			socket_,
			reinterpret_cast<const sockaddr *>(&socket_address),
			sizeof(socket_address));
	if (result == -1) {
		printf("qmp: could not connect to {}");
		close(socket_);
		return false;
	}
 
	connected_ = true;
	return true;
}
 
void qemu::qmp::disconnect() {
	if (!connected_) {
		return;
	}
 
	close(socket_);
	connected_ = false;
}
 
bool qemu::qmp::enable_cmds() const {
	if (!connected_) {
		return false;
	}
 
	std::string_view cmd { R"({ "execute": "qmp_capabilities" })" };
	return send_cmd(cmd);
}
 
bool qemu::qmp::move_mouse(int32_t delta_x, int32_t delta_y) const {
	if (!connected_) {
		return false;
	}
 
	std::string cmd {
		"{\n"
		"	\"execute\": \"input-send-event\",\n"
		"	\"arguments\": {\n"
		"		\"events\": [\n"
		"			{\n"
		"				\"type\": \"rel\",\n"
		"				\"data\": {\n"
		"					\"axis\": \"x\",\n"
		"					\"value\": " + std::to_string(delta_x) + "\n"
		"				}\n"
		"			},\n"
		"			{\n"
		"				\"type\": \"rel\",\n"
		"				\"data\": {\n"
		"					\"axis\": \"y\",\n"
		"					\"value\": " + std::to_string(delta_y) + "\n"
		"				}\n"
		"			}\n"
		"		]\n"
		"	}\n"
		"}"
	};
 
	return send_cmd(cmd);
}
 
bool qemu::qmp::send_cmd(std::string_view cmd) const {
	size_t sent = send(socket_, cmd.data(), cmd.size(), 0);
	return sent == cmd.size();
}

/* own addition */

bool qemu::qmp::mouse_down() const {
	if (!connected_) {
		return false;
	}
 
	std::string cmd {
		"{\n"
		"	\"execute\": \"input-send-event\",\n"
		"	\"arguments\": {\n"
		"		\"events\": [\n"
		"			{\n"
		"				\"type\": \"btn\",\n"
		"				\"data\": {\n"
		"					\"down\": true,\n"
		"					\"button\": \"left\"\n"
		"				}\n"
		"			}\n"
		"		]\n"
		"	}\n"
		"}"
	};
 
	return send_cmd(cmd);
}

bool qemu::qmp::mouse_up() const {
	if (!connected_) {
		return false;
	}
 
	std::string cmd {
		"{\n"
		"	\"execute\": \"input-send-event\",\n"
		"	\"arguments\": {\n"
		"		\"events\": [\n"
		"			{\n"
		"				\"type\": \"btn\",\n"
		"				\"data\": {\n"
		"					\"down\": false,\n"
		"					\"button\": \"left\"\n"
		"				}\n"
		"			}\n"
		"		]\n"
		"	}\n"
		"}"
	};
 
	return send_cmd(cmd);
}