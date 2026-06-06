#include "ical.hh"

#include <string_view>

#include <fmt/chrono.h>

#include "peglib.h" // for UTF-8 encode/decode
#include "support.hh" // for deHTML

using std::string;
using std::string_view;
using std::unordered_map;

std::string ical(const unordered_map<string, string> &act) {
  string out;

  auto emit = [&out](string_view key8, string_view val8, string_view sep8 = ":") {
    size_t anchor = out.length();
    out += key8;
    out += sep8;

    // Lines SHOULD be folded to not exceed 75 octets, but not in the middle
    // of an UTF-8 multi-octet sequence (RFC5545 3.1); go through UTF-32.
    std::u32string val32 = peg::decode(val8.data(), val8.size());

    for (char32_t ch : val32) {
      if ((out.length() - anchor) >= 70) {
        out += "\r\n ";
        anchor = out.length() - 1;
      }

      if (ch == '\n') {
        out += '\\';
        out += 'n';
      } else if (ch == ',' || ch == '\\' || ch == ';') {
        out += '\\';
        out += ch;
      } else {
        out += peg::encode_codepoint(ch);
      }
    }

    out += "\r\n";
  };

  auto emitDt = [&emit](string_view key, const string &localTimestamp) {
    time_t ts = getTstamp(localTimestamp);
    emit(key, fmt::format("{:%Y%m%dT%H%M%S}Z", fmt::gmtime(ts)));
  };

  if (act.contains("nummer") && act.contains("aanvangstijd") && act.contains("eindtijd") && act.contains("bijgewerkt")) {
    emit("BEGIN", "VCALENDAR");
    emit("VERSION", "2.0");
    emit("PRODID", "https://berthub.eu/tkconv");
    emit("BEGIN", "VEVENT");

    if (auto f = act.find("onderwerp"); f != act.end() && !f->second.empty()) {
      emit("SUMMARY", f->second);
    } else {
      emit("SUMMARY", "Activiteit " + act.at("nummer"));
    }

    if (auto f = act.find("noot"); f != act.end() && !f->second.empty()) {
      emit("DESCRIPTION", deHTML(f->second));
      emit("X-ALT-DESC;FMTTYPE=text/html", "<html><body>" + f->second + "</body></html>");
    }

    emitDt("DTSTART", act.at("aanvangstijd"));
    emitDt("DTEND", act.at("eindtijd"));
    emitDt("DTSTAMP", act.at("bijgewerkt"));

    if (auto f = act.find("zaalnaam"); f != act.end() && !f->second.empty()) {
      emit("LOCATION", f->second);
    }

    if (auto f = act.find("voortouwNaam"); f != act.end() && !f->second.empty()) {
      string cn = f->second;
      replaceSubstring(cn, "\"", "");

      // Email address is not very useful here but unfortunately mandatory
      emit("ORGANIZER", "CN=\"" + cn + "\":mailto:contact@tweedekamer.nl", ";");
    }

    emit("URL", "https://berthub.eu/tkconv/activiteit.html?nummer=" + act.at("nummer"));
    emit("UID", "https://berthub.eu/tkconv/activiteit.html?nummer=" + act.at("nummer"));

    if (auto f = act.find("soort"); f != act.end() && !f->second.empty()) {
      emit("CATEGORIES", f->second);
    }

    emit("END", "VEVENT");
    emit("END", "VCALENDAR");
  }

  return out;
}
