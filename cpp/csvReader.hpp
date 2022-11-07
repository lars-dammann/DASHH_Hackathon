#ifndef CSV_LOAD_READER
#define CSV_LOAD_READER

#include <vector>
#include "event.hpp"
#include "utils.hpp"

std::pair<std::vector<chargeTime>, std::vector<EngineEvent>>  readCSVload(const char * fname, double geoFence[4], bool dbg = false); //returns a pair of (times, loads)
#endif
