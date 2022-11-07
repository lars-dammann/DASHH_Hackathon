#include "dataSource.hpp"
#include "utils.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>

int toUnixT(std::string time){
  std::tm t{};
  std::stringstream ss(time);
  ss >> std::get_time(&t, "%d-%b-%Y %H:%M:%S");
  std::time_t timestamp = mktime(&t);
  return (int) timestamp;
}

FileSource::FileSource(std::string fname, geoFence gf) : gf(gf){
  //Assumes input file has sorted timestamps
  std::ifstream f(fname,  std::ios::in);
  if (f.is_open()) {
    std::string line;
    std::getline(f,line); // Read header

    std::string sep = "|";
    line += sep; //hackiest of hacks, but makes the tokenizer work safely.
    size_t pos = 0;
    int tPos, sogPos, latPos, lonPos, loadPos;
    int it = 0;
    while((pos = line.find(sep))!= std::string::npos ){
      std::string name = line.substr(0, pos);
      if(name == "TimeStamp") tPos = it;
      if(name == "SOG") sogPos = it;
      if(name == "LAT") latPos = it;
      if(name == "LON") lonPos = it;
      if(name == "Power") loadPos = it;
      line.erase(0,pos+sep.length());
      it++;
    }
    while (std::getline(f, line)) {
      //Split CSV with | as separator
      line += sep;
      std::vector<std::string> tokens;
      while((pos = line.find(sep))!= std::string::npos ){
        tokens.push_back(line.substr(0, pos));
        line.erase(0,pos+sep.length());
      }
      if(tokens.size() <= loadPos) continue;
      auto t = toUnixT(tokens[tPos]);
      auto sog = std::stof(tokens[sogPos]);
      auto lat = std::stof(tokens[latPos]);
      auto lon = std::stof(tokens[lonPos]);
      auto load = std::stof(tokens[loadPos]);
      data.push_back({t,sog,lat,lon,load});
    }
    f.close();
  }
  else {
    std::cerr << "Unable to open file\n";
  }
  if(data.size() == 0){
    std::cerr << "Loading file produced no data" << std::endl;}
  else{
    std::cout << "Loading file was successfull" << std::endl;
  }
}

float FileSource::getLoad(int timePoint){
  int offset = data[0].time;
  timePoint += offset; //for API purposes all time is to be indexed from 0
  auto pt = std::lower_bound(data.begin(), data.end(), timePoint, [](const dataPoint& dp, int t){return dp.time < t;});
  if(pt == data.end())
    return 0;
  return (float) pt->load;
}

bool FileSource::canCharge(int timePoint){
  int offset = data[0].time;
  timePoint += offset; //for API purposes all time is to be indexed from 0
  auto pt = std::lower_bound(data.begin(), data.end(), timePoint, [](const dataPoint& dp, int t){return dp.time < t;});
  if(pt == data.end()) return false;
  bool inFence = pt->Lat >= gf.minLat && pt->Lat <= gf.maxLat && pt->Lon >= gf.minLon && pt->Lon <= gf.maxLon;
  return inFence && pt->SOG <= 1e-4;
}
