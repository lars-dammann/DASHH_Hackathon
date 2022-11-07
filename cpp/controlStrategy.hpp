#ifndef CONTROLSTRATEGY_HPP
#define CONTROLSTRATEGY_HPP
struct Ship;

#include <iostream>
#include <functional>
#include <gsl/gsl_multimin.h>
#include "components.hpp"

#define FATAL std::cout << "You have managed to call a method of the control strategy base class. Please do not do that. Ever.\n";

class controlStrategy {
public:
  virtual bool setUp(const gsl_vector* v, int offset){FATAL; return false;} //Base control strategy can never be valid target for optimisation
  virtual int getParamCount() const {FATAL; return 0;}
  virtual std::vector<float> getParamList() const {FATAL; return {};}
  virtual float getEnginePower (const Ship& S, float load) const {FATAL; return 0;}
  virtual ~controlStrategy() = default;
};

class rangeExtender : public controlStrategy {
private:
  float upperSoC = 0.5f;
  float lowerSoC = 0.15f;
public:
  rangeExtender() = default;
  float getEnginePower(const Ship& S, float load) const;
  int getParamCount() const {return 2;}
  std::vector<float> getParamList() const {return {lowerSoC, upperSoC};}
  bool setUp(const gsl_vector* v, int offset){

    float lo = (float) gsl_vector_get(v, offset+0);
    float hi = (float) gsl_vector_get(v, offset+1);

    if(lo >= hi || hi > 0.9 || lo < 0.05) return false;
    upperSoC = hi;
    lowerSoC = lo;
    return true;
  }
};



class peakShaver : public controlStrategy {
public:
  float getEnginePower (const Ship& S, float load) const;
};

#endif
