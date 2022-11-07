#include "event.hpp"

int getCurLoad(std::vector<EngineEvent>* events, int time){ //Obtain the load at some time. Assume event are recorded only on a change of load & time starts at 0
  if(time <= 0) return 0;
  if(time >= (*events)[events->size()-1].time) return 0;
  auto ev = std::upper_bound(events->begin(), events->end(), time, [](int tm, EngineEvent& ev){return tm < ev.time ;});
  return (ev-1)->load;
}

bool isCharging(std::vector<EngineEvent>* events, int time, int ConnectTime){
  if(time <= 0) return false;
  if(time >= (*events)[events->size()-1].time) return false;
  auto ev = std::upper_bound(events->begin(), events->end(), time, [](int tm, EngineEvent& ev){return tm < ev.time ;});
  return ((ev-1)->load == 0 && (ev-1)->time < time - ConnectTime);
}
