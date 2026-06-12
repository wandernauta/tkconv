#pragma once

#include <string>
#include <unordered_map>

/*
Geef een beschrijving van een activiteit in iCalendar-formaat, geschikt voor
gebruik in agendasoftware.

De velden aanvangstijd, bijgewerkt, eindtijd, nummer zijn verplicht.

De velden noot, onderwerp, soort, voortouwNaam, zaalnaam zijn optioneel.
*/
std::string ical(const std::unordered_map<std::string, std::string> &activiteit);
