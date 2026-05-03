#include "keyboard.hh"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <chrono>

const std::map<std::string, ZoneData> ZONES = {
//  ZONE     INIT  COMMIT  STATE 
  {"left",  {0x51,   0x58,  0x52}}, 
  {"mid",   {0x41,   0x48,  0x42}},
  {"right", {0x31,   0x38,  0x32}},
  {"all",   {0x01,   0x08,  0x02}}
};

const std::map<std::string, uint8_t rgb_val[3]> preset_colors = {
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
    std::cerr << "Start the app as root." << std::endl;
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
  { std::cerr << "Write failed at 0x" << std::hex << offset << std::endl; }
}

void KeyboardController::trigger_latch(const ZoneData& zone)
{
  write_ec(REG_ZONE, zone.init);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  
  write_ec(REG_ZONE, zone.commit);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  
  write_ec(REG_ZONE, zone.state);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void KeyboardController::set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, const std::string& zone_name)
{
  if(ZONES.find(zone_name) == ZONES.end()) { return; }

  if(current_mode != "solid") { set_mode("solid"); }

  write_ec(REG_RED, r);
  write_ec(REG_GREEN, g);
  write_ec(REG_BLUE, b);
  write_ec(REG_PWM, brightness);
  
  trigger_latch(ZONES.at(zone_name));
}

void KeyboardController::set_mode(const std::string& mode_name)
{
  if(mode_name == "solid")
  { write_ec(REG_MODE_1, 0x01); write_ec(REG_MODE_2, 0x01); write_ec(REG_MODE_CMD, 0x11); }

  else if(mode_name == "breathe")
  { write_ec(REG_MODE_1, 0x03); write_ec(REG_MODE_2, 0x03); write_ec(REG_MODE_CMD, 0x31); }

  else if(mode_name == "cycle")
  { write_ec(REG_MODE_1, 0x06); write_ec(REG_MODE_2, 0x06); write_ec(REG_MODE_CMD, 0x61); }

  current_mode = mode_name;
}

void KeyboardController::restore_original()
{ set_color(orig_state.r, orig_state.g, orig_state.b, orig_state.pwm, "all"); }
