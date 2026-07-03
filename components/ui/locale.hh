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
  std::string gpu_warning;
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
  "Uygula",
  "Sakın terminalde rebootlayabileceğine dair bir mesaj çıkana kadar uygulamayı kapatma. Aksi takdirde sorumluluk kabul etmiyorum."
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
  "Apply",
  "Don't ever close the app until terminal says you can reboot. Otherwise, I accept no responsibility."
};

#endif
