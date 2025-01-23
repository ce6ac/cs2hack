#include "qmp.h"
 
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#include <spdlog/spdlog.h>
 
qemu::QMP::QMP()
    : connected_(false),
      socket_(0) {}
 
bool qemu::QMP::Connect(std::string_view address, uint32_t port) {
  if (connected_) {
    spdlog::warn("connection is already open");
    printf("connection is already open");
    return true;
  }
 
  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_ == -1) {
    spdlog::critical("could not create socket descriptor");
    printf("could not create socket descriptor");
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
    spdlog::critical("could not connect to {}", address);
    printf("could not connect to {}");
    close(socket_);
    return false;
  }
 
  connected_ = true;
  return true;
}
 
void qemu::QMP::Disconnect() {
  if (!connected_) {
    return;
  }
 
  close(socket_);
  connected_ = false;
}
 
bool qemu::QMP::EnableCommands() const {
  if (!connected_) {
    return false;
  }
 
  std::string_view message{R"({ "execute": "qmp_capabilities" })"};
  return Send(message);
}
 
bool qemu::QMP::MoveMouse(int32_t delta_x, int32_t delta_y) const {
  if (!connected_) {
    return false;
  }
 
  std::string message{
    "{\n"
    "  \"execute\": \"input-send-event\",\n"
    "  \"arguments\": {\n"
    "    \"events\": [\n"
    "      {\n"
    "        \"type\": \"rel\",\n"
    "        \"data\": {\n"
    "          \"axis\": \"x\",\n"
    "          \"value\": " + std::to_string(delta_x) + "\n"
    "        }\n"
    "      },\n"
    "      {\n"
    "        \"type\": \"rel\",\n"
    "        \"data\": {\n"
    "          \"axis\": \"y\",\n"
    "          \"value\": " + std::to_string(delta_y) + "\n"
    "        }\n"
    "      }\n"
    "    ]\n"
    "  }\n"
    "}"
  };
 
  return Send(message);
}
 
bool qemu::QMP::Send(std::string_view message) const {
  size_t sent = send(socket_, message.data(), message.size(), 0);
  return sent == message.size();
}