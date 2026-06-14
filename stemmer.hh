#pragma once

#include <sqlite3.h>

// Voor gebruik met sqlite3_auto_extension
int installDutch(sqlite3 *db, char **pzErrMsg, const struct sqlite3_api_routines *pThunk);
