#include "csvReader.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>


std::pair<std::vector<chargeTime>, std::vector<EngineEvent>>  readCSVload(const char * fname, double geoFence[4], bool dbg){
  if(dbg) std::cout << "Attempting to read file " << fname << std::endl;
  std::vector<EngineEvent> events;
  std::vector<std::tuple<int, double, double>> chargeEvents;

  std::ifstream fs(fname, std::ios::in);
  if (fs.is_open()) {
    std::string line;
    int numEngineEVs;
    int numChargeEVs;
    std::getline(fs, line);
    std::stringstream ss{line};
    ss >> numEngineEVs >> numChargeEVs;
    if(dbg) std::cout << "File contains " << numEngineEVs << " engine events and " << numChargeEVs << " charging events." << std::endl;
    for(int i = 0; i < numEngineEVs; i++){
      std::getline(fs, line);
      std::stringstream ss{line};
      int time;
      int load;
      ss >> time >> load;
      events.push_back({time, load});
      if(dbg) std::cout << time << ": " << load << std::endl;
    }
    for(int i = 0; i < numChargeEVs; i++){
      std::getline(fs, line);
      std::stringstream ss{line};
      int time;
      double lat;
      double lon;
      ss >> time >> lat >> lon;
      chargeEvents.push_back({time, lat, lon});
    }

    fs.close();
  }
  else {
    std::cerr << "Unable to open file\n";
  }

  auto chargeTimes = getChargeTimes(chargeEvents, geoFence);
  int offset = 0;
  if(events.size() > 0){
    std::sort(events.begin(), events.end(), [](const EngineEvent& a, const EngineEvent& b){return a.time < b.time;});
    offset = events[0].time;
    for(auto& ev : events){ ev.time -= offset; ev.load *= 875. / 100.;}
  }

  if(chargeTimes.size() > 0){
    std::sort(chargeTimes.begin(), chargeTimes.end(), [](const chargeTime& a, const chargeTime& b){return a.first < b.first; });
    for(auto& it : chargeTimes){
      it.first -= offset;
      it.second -= offset;
    }
  }


  return {chargeTimes, events};
}
