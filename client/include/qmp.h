#ifndef DAEMON_SRC_QEMU_QMP_H_
#define DAEMON_SRC_QEMU_QMP_H_
 
#include <cstdint>
#include <string_view>
 
namespace qemu {
/**
 * Basic interface for the QEMU Machine Protocol: a JSON-based protocol which
 * allows applications to control a QEMU instance.
 */
class QMP {
 public:
  QMP();
 
  bool Connect(std::string_view address, uint32_t port);
 
  void Disconnect();
 
  bool EnableCommands() const;
 
  bool MoveMouse(int32_t delta_x, int32_t delta_y) const;
 
 private:
  bool connected_;
  int32_t socket_;
 
  bool Send(std::string_view message) const;
};
}
 
#endif //DAEMON_SRC_QEMU_QMP_H_