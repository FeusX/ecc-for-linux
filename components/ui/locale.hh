#ifndef LOCALE_HH
#define LOCALE_HH

#include <string>

struct Locale {
  std::string zone_all;
  std::string zone_left;
  std::string zone_right;
  std::string zone_mid;
  std::string pattern_solid;
  std::string pattern_cycle;
  std::string pattern_pulse;
  std::string pattern_ambilight;
  std::string btn_apply;
};

const Locale TR = {
  "Hepsi",
  "Sol",
  "Sağ",
  "Orta",
  "Durağan",
  "Döngü",
  "Dalga",
  "Ambiyans",
  "Uygula"
};

const Locale EN = {
  "All",
  "Left",
  "Right",
  "Middle",
  "Solid",
  "Cycle",
  "Pulse",
  "Ambilight",
  "Apply"
};

#endif
