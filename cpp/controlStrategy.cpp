#include "controlStrategy.hpp"



float rangeExtender::getEnginePower(const Ship& S, float load) const {
  if(S.engine->PwrMax == 0) return 0.f;
  if(S.state.landPwr > 0) return 0.f;
  float minLoadToProvide = 0;

  // ToDos for tomorrow
  // Visualisation, Writeup of simulation, Writeup of batterystrategys

  // If SoC is below the DischargeLimit, we provide the full load
  // Not entirely clear to me what this means.
  if(S.battery->DischargeLimitSoC > S.state.charge) minLoadToProvide = load;

  // If charger is above upperbound, we put engine at minimum
  if(S.state.charge > upperSoC) return minLoadToProvide;

  // If charge is below the charge lowerbound, we put the engine at maximum power
  if(S.state.charge < lowerSoC) return S.engine->PwrMax;

  // If engine was running before, we let it run at full power
  if(S.state.engPwr) return S.engine->PwrMax;
  return minLoadToProvide;
}

float peakShaver::getEnginePower(const Ship& S, float load) const {

  return 0;
}
