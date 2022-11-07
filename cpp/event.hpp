#ifndef EVENT_HPP
#define EVENT_HPP

#include <algorithm>
#include <vector>

struct EngineEvent{
  int time;
  int load;
};

int getCurLoad(std::vector<EngineEvent>* events, int time);
bool isCharging(std::vector<EngineEvent>* events, int time, int ConnectTime);
#endif
