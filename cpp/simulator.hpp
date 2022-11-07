#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "performanceModel.hpp"
#include "controlStrategy.hpp"
#include "components.hpp"
#include "dataSource.hpp"
#include "utils.hpp"

#include <gsl/gsl_multimin.h>
#include <vector>
#define ITERATION_CUTOFF 40
constexpr float eps = 1e-1;

template <typename DataSourceType>
std::pair<SimResults, std::vector<Record>>simulate(Ship& ship, DataSourceType& data, bool recordSim = true){
  SimResults results;
  Record r;
  r.charge = 0.2f; //initally, battery is charged to 20%
  ship.state = r;
  std::vector<Record> rvec;
  if(recordSim) rvec.push_back(r);

  int timeSincePlugin = 0;
  int dt = data.getResolution();
  int time = 0;

  for(auto[load, canCharge] : data){

    Record currentRecord;
    currentRecord.load = load;
    currentRecord.time = time;
    currentRecord.charge = ship.state.charge;

    time += dt;
    results.TotalTime += dt;
    //############################ Simulation Begin ################

    // Determine whether ship can charge, and whether it wants to charge
    if(canCharge){
      timeSincePlugin += dt;
    }else{
      timeSincePlugin = 0;
    }

    if(timeSincePlugin >= ship.chargePoint->pluginDelay){
      if(ship.state.charge < ship.battery->ChargeLimitSoC){

        //Plugged in for long enough, battery not at charge limit, so we charge the battery.
        float addedCharge = ship.chargePoint->powerLimit * dt / secsPerHour ;
        float addedChargePercentage = addedCharge / ship.BatteryCap;
        currentRecord.landPwr = (float) ship.chargePoint->powerLimit;
        currentRecord.charge += addedChargePercentage;
        if(addedCharge > 0){
          results.TimeShareLand += dt;
        }else{
          results.TimeShareRest += dt;
        }
        results.EnergyShareLand += addedCharge;
        results.loadCycles += addedChargePercentage;
      }else{
        results.TimeShareRest += dt;
      }
    }else if(timeSincePlugin > 0){
      results.TimeShareRest += dt;
    }

    //Process distribution of loads
    float EnginePwr = ship.ctrl->getEnginePower(ship, load);
    float BatteryPwr = load - EnginePwr;
    //if(BatteryPwr < 0) BatteryPwr *= ship.battery->etaBat; // Efficiency loss while charging battery
    //if(BatteryPwr > ship.battery->maxCRateDischargeCont * ship.BatteryCap) BatteryPwr = ship.battery->maxCRateDischargeCont * ship.BatteryCap; //Limit battery power supply to C-Rate
    currentRecord.engPwr = EnginePwr;
    if(ship.state.charge <= ship.battery->DischargeLimitSoC && BatteryPwr > 0) BatteryPwr = 0;//Battery empty no more power supplied
    currentRecord.batPwr = BatteryPwr;

    //Select what the current state of power supply is. If the ship is charging, this already happened at the charging computation
    if(!canCharge){
      if(BatteryPwr + EnginePwr < load){
        //Record out of power status
        currentRecord.powerDeficit = load - BatteryPwr - EnginePwr;
        results.InsufPwrT += dt;
      }else{
        //Sufficient power, not at charging station, record what the ship is doing
        bool engineRunning = false;
        if(EnginePwr >= eps){
          engineRunning = true;
        }
        bool batteryDischarging = false;
        if(BatteryPwr >= eps){
          batteryDischarging = true;
        }
        bool batteryCharging = false;
        if(BatteryPwr <= -eps){
          batteryCharging = true;
        }
        bool batteryIdle = !batteryCharging && !batteryDischarging;

        if( engineRunning &&  batteryDischarging) results.TimeShareHybrid  += dt;
        if(!engineRunning &&  batteryDischarging) results.TimeShareBat     += dt;
        if( engineRunning && !batteryDischarging) results.TimeShareEng     += dt;
        if(!engineRunning && !batteryDischarging) results.TimeShareRest    += dt;
      }
    }

    results.EnergyShareFuel += (float) EnginePwr * dt/secsPerHour;
    results.FuelConsumption += ((float) EnginePwr * dt/secsPerHour);
    //results.FuelConsumption += ((float) EnginePwr * dt/secsPerHour) / ship.engine->efficiency((int) EnginePwr);
    float chargeChange = (BatteryPwr * dt / secsPerHour) / ship.BatteryCap;
    currentRecord.charge -= chargeChange;
    //############################ Simulation End ################
    if(recordSim) rvec.push_back(currentRecord);
    ship.state = currentRecord;
  }
  float loadCyclesPerYear = results.loadCycles * secsPerYear / results.TotalTime;
  int   batteriesNeeded = (int) std::ceil( loadCyclesPerYear *  ship.targetLifetime / ship.battery->BatCycles);
  float normalisation = secsPerYear / results.TotalTime  * ship.targetLifetime;

  results.CO2Backpack = batteriesNeeded * ship.BatteryCap * ship.battery->BatteryCO2BackpackperKWH + ship.engine->EnergyConverterCO2Backpack;
  results.CO2Operational = results.FuelConsumption * ship.engine->FuelC02EmissionsperKWH;
  results.CO2Operational += results.EnergyShareLand * ship.chargePoint->electricityCO2EmissionperKWH / ship.battery->etaBat;
  results.CO2Operational *= normalisation;
  results.CO2Total = results.CO2Operational + results.CO2Backpack;

  results.CostBatteryperLifetime = batteriesNeeded * ship.BatteryCap * ship.battery->BatteryPricePerKWh;
  results.CostFuelperLifeTime = results.FuelConsumption * ship.engine->FuelPriceperKWH * normalisation;
  results.CostElectricityperLifeTime = results.EnergyShareLand * ship.chargePoint->electricityPriceperKWh * normalisation / ship.battery->etaBat;

  results.Capex = results.CostBatteryperLifetime + ship.installedEngines*ship.engine->EnergyConverterPrice + ship.chargePoint->Capex;
  results.Opex = results.CostElectricityperLifeTime + results.CostFuelperLifeTime;
  results.TCO = results.Capex + results.Opex;


  return {results, rvec};
}



constexpr double InvalidConfigCost = 1e9;
template <typename DataSourceType>
std::pair<Ship, SimResults> optimiseShip(Ship& s, DataSourceType& data){

  auto evaluationFunction = [](const gsl_vector* v, void* params){
      float batCap = (float) gsl_vector_get(v,0);
      auto [s, data] = *((std::pair<Ship*, DataSourceType*>*) params);
      if(batCap > s->battery->maxCap || batCap < s->battery->minCap) return InvalidConfigCost;
      s->BatteryCap = batCap;
      if( !s->ctrl->setUp(v, 1) ) return InvalidConfigCost;
      auto [res, discard] = simulate(*s, *data, false);
      return (double) res.TCO + res.InsufPwrT * 1000; //Or whatever evaluation is deemed appropriate, currently only considering capital cost + some penalty for being out of power
  };

  const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;

  gsl_multimin_fminimizer *minimizer = NULL;
  gsl_vector *simplexSize, *x;
  gsl_multimin_function funcWrapper;

  size_t iter = 0;
  int status;
  double size;

  int dim = 1 + s.ctrl->getParamCount();

  x = gsl_vector_alloc(dim); //Allocate one dimension for battery cap and however many the control strategy needs
  gsl_vector_set(x, 0, (s.battery->maxCap + s.battery->minCap)/2.); //start at "medium" capacity
  for(int i = 0; i < dim-1; i++)
    gsl_vector_set(x, i+1, s.ctrl->getParamList()[i]);

  simplexSize = gsl_vector_alloc(dim);
  gsl_vector_set(simplexSize, 0, (s.battery->maxCap + s.battery->minCap)/2.);
  for(int i = 0; i < dim-1; i++)
    gsl_vector_set(simplexSize, i+1, .3);

  funcWrapper.n = dim;
  funcWrapper.f = evaluationFunction;
  std::pair<Ship*, DataSourceType*> params = {&s, &data};
  funcWrapper.params = &params;

  minimizer = gsl_multimin_fminimizer_alloc (T, dim);
  gsl_multimin_fminimizer_set (minimizer, &funcWrapper, x, simplexSize);


  do {
    iter++;
    status = gsl_multimin_fminimizer_iterate(minimizer);
    if(status) break;

    size = gsl_multimin_fminimizer_size (minimizer);
    status = gsl_multimin_test_size (size, 0.1);
    if(status == GSL_SUCCESS) break;

  } while(status = GSL_CONTINUE && iter < ITERATION_CUTOFF);

  gsl_vector_free(x);
  gsl_vector_free(simplexSize);
  gsl_multimin_fminimizer_free (minimizer);

  auto [simRes, disc] = simulate(s, data);
  return {s, simRes};
}

template <typename DataSourceType>
std::vector<std::pair<Ship, SimResults>> optimisePortfolio( DataSourceType& data,
                                        std::vector<std::shared_ptr<Battery>>& bats,
                                        std::vector<std::shared_ptr<EnergyConverter>>& engs,
                                        std::vector<std::shared_ptr<ChargePoint>>& cps,
                                        std::vector<std::shared_ptr<controlStrategy>>& ctrls,
                                        float targetLifetime = 20.f
                                        )
{
  std::vector<std::pair<Ship, SimResults>> rval;
  //setup all configurations
  for(auto& b : bats){
    for(auto& e : engs){
      for(auto& cp : cps){
        for(auto& ctrl : ctrls){
          for(int engcount = e->minCount; engcount <= e->maxCount; engcount++){
            auto tmpCtrl = std::make_shared<rangeExtender>(*std::static_pointer_cast<rangeExtender>(ctrl)); //copy control strategy to make changes to it, in future we need to actually detect what ctrl strat is in use here!!
            Ship s;
            s.battery = b;
            s.engine = e;
            s.chargePoint = cp;
            s.ctrl = tmpCtrl;
            s.installedEngines = engcount;
            s.targetLifetime = targetLifetime;
            rval.push_back( optimiseShip(s, data) );
          }
        }
      }
    }
  }
  return rval;
}
#endif
