#include "keyboard.hh"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <array>

const std::map<std::string, std::array<uint8_t, 3>> preset_colors = {
  {"red",    {255, 0, 0}},
  {"green",  {0, 255, 0}},
  {"blue",   {0, 0, 255}}
};

KeyboardController::KeyboardController()
{ ec_fd = open(EC_PATH, O_RDWR); }

KeyboardController::~KeyboardController()
{ if(ec_fd != -1) close(ec_fd); }

bool KeyboardController::init()
{
  if(ec_fd == -1) // check if root
  {
    std::cerr << "[WARNING] Ec I/O not found. Trying to load ec_sys..." << std::endl;
    std::system("modprobe ec_sys write_support=1");
    std::cout << "[LOG] ec_sys loaded." << std::endl;

    ec_fd = open(EC_PATH, O_RDWR);
  }

  if(ec_fd == -1)
  {
    std::cerr << "[ERROR] Cannot access Embedded Controller. Please run as root." << std::endl;
    return false;
  }
  
  orig_state.r = read_ec(REG_RED);
  orig_state.g = read_ec(REG_GREEN);
  orig_state.b = read_ec(REG_BLUE);
  orig_state.pwm = read_ec(REG_PWM);
  return true;
}

uint8_t KeyboardController::read_ec(uint16_t offset)
{
  uint8_t val = 0;
  lseek(ec_fd, offset, SEEK_SET);
  read(ec_fd, &val, 1);
  return val;
}

void KeyboardController::write_ec(uint16_t offset, uint8_t val)
{
  lseek(ec_fd, offset, SEEK_SET);
  if(write(ec_fd, &val, 1) != 1)
  { std::cerr << "[ERROR] Write failed at 0x" << std::hex << offset << std::dec << std::endl; }
}

void KeyboardController::trigger_latch(ZoneID zone)
{
  uint8_t base = static_cast<uint8_t>(zone);

  // init byte ends with 0x01
  write_ec(REG_ZONE, base | 0x01);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // commit byte ends with 0x08  
  write_ec(REG_ZONE, base | 0x08);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // state byte ends with 0x02
  write_ec(REG_ZONE, base | 0x02);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void KeyboardController::set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, ZoneID zone)
{
  std::lock_guard<std::mutex> lock(ec_mutex);

  if(current_pattern != KBPattern::SOLID) { set_mode_internal(KBPattern::SOLID); }
  
  write_ec(REG_RED, r);
  write_ec(REG_GREEN, g);
  write_ec(REG_BLUE, b);
  write_ec(REG_PWM, brightness);

  trigger_latch(zone);
}

void KeyboardController::set_mode_internal(KBPattern p)
{
  write_ec(REG_MODE_1, static_cast<uint8_t>(p));
  write_ec(REG_MODE_2, static_cast<uint8_t>(p));
  write_ec(REG_MODE_CMD, (static_cast<uint8_t>(p) << 4) | 0x01);

  current_pattern = p;
}

void KeyboardController::set_mode(KBPattern p)
{
  std::lock_guard<std::mutex> lock(ec_mutex);
  set_mode_internal(p);
}

void KeyboardController::restore_original()
{ set_color(orig_state.r, orig_state.g, orig_state.b, orig_state.pwm, ZoneID::ALL); }
