#ifndef DAEMON_SRC_QEMU_QMP_H_
#define DAEMON_SRC_QEMU_QMP_H_
 
#include <cstdint>
#include <string_view>
 
namespace qemu {
/**
 * Basic interface for the QEMU Machine Protocol: a JSON-based protocol which
 * allows applications to control a QEMU instance.
 */
class qmp {
 public:
  qmp();
 
  bool setup(std::string_view address, uint32_t port);
 
  void disconnect();
 
  bool enable_cmds() const;
 
  bool move_mouse(int32_t delta_x, int32_t delta_y) const;

  /* own addition */

  bool mouse_down() const;

  bool mouse_up() const;
 
 private:
  bool connected_;
  int32_t socket_;
 
  bool send_cmd(std::string_view message) const;
};
}
 
#endif //DAEMON_SRC_QEMU_QMP_H_