#include "doctest.h"
#include <string>

#include "ical.hh"

using std::string;
using std::unordered_map;

TEST_CASE("iCal voor lege activiteit") {
  CHECK_EQ(ical({}), "");
}

TEST_CASE("iCal voor representatieve activiteit") {
  unordered_map<string, string> activiteit = {
    {"nummer", "2026A03861"},
    {"aanvangstijd", "2026-05-15T12:00:00"},
    {"eindtijd", "2026-05-15T12:00:00"},
    {"soort", "E-mailprocedure"},
    {"onderwerp", "Verzoek van het lid Neijenhuis (D66), mede namens VVD, GroenLinks-PvdA, CDA en JA21, tot uitstel van de inbrengdatum voor het verslag over het wetsvoorstel Wet BAZ (36912)"},
    {"noot", "<br />In reactie op onderstaande e-mailprocedure met een verzoek van het lid Neijenhuis (D66)..."},
    {"voortouwNaam", "vaste commissie voor Sociale Zaken en Werkgelegenheid"},
    {"zaalnaam", "Fictieve voorbeeldzaal"},
    {"bijgewerkt", "2026-04-10T10:55:25.3870000"}
  };

  string expected =
    "BEGIN:VCALENDAR\r\n"
    "VERSION:2.0\r\n"
    "PRODID:https://berthub.eu/tkconv\r\n"
    "BEGIN:VEVENT\r\n"
    "SUMMARY:Verzoek van het lid Neijenhuis (D66)\\, mede namens VVD\\, Groen\r\n"
    " Links-PvdA\\, CDA en JA21\\, tot uitstel van de inbrengdatum voor het v\r\n"
    " erslag over het wetsvoorstel Wet BAZ (36912)\r\n"
    "DESCRIPTION: In reactie op onderstaande e-mailprocedure met een verzoe\r\n"
    " k van het lid Neijenhuis (D66)...\r\n"
    "X-ALT-DESC;FMTTYPE=text/html:<html><body><br />In reactie op onderstaa\r\n"
    " nde e-mailprocedure met een verzoek van het lid Neijenhuis (D66)...</\r\n"
    " body></html>\r\n"
    "DTSTART:20260515T100000Z\r\n"
    "DTEND:20260515T100000Z\r\n"
    "DTSTAMP:20260410T085525Z\r\n"
    "LOCATION:Fictieve voorbeeldzaal\r\n"
    "ORGANIZER;CN=\"vaste commissie voor Sociale Zaken en Werkgelegenheid\":m\r\n"
    " ailto:contact@tweedekamer.nl\r\n"
    "URL:https://berthub.eu/tkconv/activiteit.html?nummer=2026A03861\r\n"
    "UID:https://berthub.eu/tkconv/activiteit.html?nummer=2026A03861\r\n"
    "CATEGORIES:E-mailprocedure\r\n"
    "END:VEVENT\r\n"
    "END:VCALENDAR\r\n";

  string actual = ical(activiteit);

  CHECK_EQ(expected, actual);
}

TEST_CASE("iCal met UTF-8 randgeval") {
  unordered_map<string, string> activiteit = {
    {"nummer", "1900A0001"},
    {"aanvangstijd", "2026-05-15T12:00:00"},
    {"eindtijd", "2026-05-15T12:00:00"},
    {"zaalnaam", "ééééééééééééééééééééééééééééééééé"},
    {"bijgewerkt", "2026-04-10T10:55:25.3870000"}
  };

  for (int i = 0; i < 4; i++) {
    string actual = ical(activiteit);
    size_t loc = actual.find("LOCATION:");

    // Het resultaat moet een LOCATION bevatten
    REQUIRE_NE(loc, std::string::npos);

    // Regel mag niet langer zijn dan 75 octets
    REQUIRE_NE(actual.find("\r\n", loc), std::string::npos);
    CHECK_LT(actual.find("\r\n", loc) - loc, 75);

    // De beide bytes van é mogen niet zijn opgeknipt...
    CHECK_NE(actual.find("é\r\n é", loc), std::string::npos);

    // ...ook niet als we het geheel één positie opschuiven
    activiteit["zaalnaam"].insert(0, " ");
  }
}

TEST_CASE("iCal met embedded newline") {
  unordered_map<string, string> activiteit = {
    {"nummer", "1900A0002"},
    {"aanvangstijd", "2026-05-16T12:00:00"},
    {"eindtijd", "2026-05-16T12:00:00"},
    {"zaalnaam", "A \n B"},
    {"bijgewerkt", "2026-04-10T11:55:25.3870000"}
  };

  string actual = ical(activiteit);

  CHECK_NE(actual.find("A \\n B"), std::string::npos);
}
