#ifndef KEYBOARD_HH
#define KEYBOARD_HH

#include <string>
#include <map>
#include <cstdint>
#include <mutex>

enum class ZoneID : uint8_t { ALL = 0x00, LEFT = 0x50, MID = 0x40, RIGHT = 0x30 };

enum class KBPattern : uint8_t { SOLID = 0x01, BREATHE = 0x03, CYCLE = 0x06, AMBILIGHT = 0x07 };

class KeyboardController {
public:
  KeyboardController();
  ~KeyboardController();

  bool init();
  void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, ZoneID zone);
  void commit_color();
  void set_mode(KBPattern p);
  void restore_original();

private:
  static constexpr const char* EC_PATH = "/sys/kernel/debug/ec/ec0/io"; // this is linux embedded controller path
  
  static constexpr uint16_t REG_RED = 0x89;
  static constexpr uint16_t REG_GREEN = 0x8A;
  static constexpr uint16_t REG_BLUE = 0x8B;
  static constexpr uint16_t REG_PWM = 0x64;
  static constexpr uint16_t REG_ZONE = 0xAC;

  static constexpr uint16_t REG_MODE_CMD = 0xC1; 
  static constexpr uint16_t REG_MODE_1 = 0xEB;
  static constexpr uint16_t REG_MODE_2 = 0xE2;

  int ec_fd = -1;
  std::mutex ec_mutex;

  KBPattern current_pattern = KBPattern::SOLID;
    
  struct {
    uint8_t r, g, b, pwm;
  } orig_state;

  uint8_t read_ec(uint16_t offset);
  void write_ec(uint16_t offset, uint8_t val);
  void trigger_latch(ZoneID zone);
  void set_mode_internal(KBPattern p);
};

#endif
