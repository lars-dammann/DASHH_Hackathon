#ifndef PERFORMANCE_MODEL_HPP
#define PERFORMANCE_MODEL_HPP

#include <random>
#include <algorithm>
#include <iostream>
#include <functional>

#include "event.hpp"
#include "utils.hpp"

class FileSource;
class GenericSource;

static std::random_device ranDev;
static std::mt19937 Generator{ranDev()};
static auto rng01 = std::bind(std::uniform_real_distribution<double>{0,1}, Generator);

//static auto RateOfChange = std::bind(std::exponential_distribution<> {}, Generator);


struct MarkovPhase {
  int startTime = 0;
  int endTime = 0;
  int id = -1; //invalid default
};

struct Profile{
  double avgPwr = -1;        // average power draw during regular operation in kW
  double stdPwr = 1;        // standard deviation power draw is assumed as a normal distribution

  double   avgTme = 7200;    // total activity time in seconds
  double   stdTme = 1800;    // standard deviation of activity time

  int   timeToCompletion = 0;      // If Profile is being sampled, counts active time

  double curPower = 0;

  std::function<double()> powerDist;
  std::function<double()> timeDist;


  Profile(){timeToCompletion = 0; initDistributions();}

  void initDistributions(){
    stdPwr = std::clamp(stdPwr, 1e-5, 500.);
    stdTme = std::clamp(stdTme, 1e-5, 100000.);

    powerDist       = std::bind(std::normal_distribution<> {avgPwr, stdPwr}, Generator);
    if(stdTme < 0.5 * avgTme){
      timeDist        = std::bind(std::normal_distribution<> {avgTme, stdTme}, Generator);
    }else{
      timeDist        = std::bind(std::exponential_distribution<> {1 / avgTme}, Generator);
    }

  }

  void printInfo(){
    std::cout << "~~~~~~~Performance Profile~~~~~~~" << std::endl;
    std::cout << "Average active time: " << avgTme << " with deviation " << stdTme << std::endl;
    std::cout << "Average power draw:  " << avgPwr << " with deviation " << stdPwr << std::endl;
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << "Currently " << timeToCompletion << " seconds left to completion" << std::endl;

  }
  bool isActive(){return (timeToCompletion > 0);}
  void activate(){
    curPower = avgPwr;
    timeToCompletion = (int) std::round(timeDist());
  }
  void deactivate(){timeToCompletion = 0;}
  double  sample(int timeStep = 1){
    if(!isActive()) return 0;
    double draw = 0;

    timeToCompletion -= timeStep;


    int pw = (int) std::clamp( std::round(powerDist()), avgPwr - 3*stdPwr, avgPwr + 3*stdPwr);
    if(pw < 0) pw = -1;

    curPower = (0.95) * curPower + 0.05 *  pw;

    if(timeToCompletion <= 0) deactivate();
    return curPower;
  }
};

class performanceModel {
private:
  int timeResolution = 4;
  int numProfiles;
  int activeProfile = 0;

  std::vector< std::vector< float > > transitionMatrix;
  std::vector< Profile* > profiles;

  void resetTransition(){for(auto& row : transitionMatrix)for(auto&x : row) x = 0;};
  void fitTransitionMatrix(std::vector<MarkovPhase>& phases);


public:
  void transition();
  void info();
  inline bool isCharging(){return activeProfile >= timeResolution*numProfiles-timeResolution;}
  int sample(int timeStep);
  void matchToData(std::vector<EngineEvent>& loads, std::vector<chargeTime>& chargingTimes, bool dbg = false);
  void matchToData(GenericSource& fs);
  void matchToData(FileSource& fs);
  performanceModel (int numProfiles, int timeResolution);
  performanceModel ();
  virtual ~performanceModel ();
};


#endif
