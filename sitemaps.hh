#pragma once
#include "thingpool.hh"
#include "sws.hh"
#include "sqlwriter.hh"

void createSiteMapHandlers(SimpleWebSystem& sws, ThingPool<SQLiteWriter>& tp);
