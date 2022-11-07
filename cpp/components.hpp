#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "controlStrategy.hpp"
#include "utils.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <iomanip>


class controlStrategy;

struct Record{
  int time = 0;
  float load = 0.f;
  float engPwr = 0.f;
  float batPwr = 0.f;
  float landPwr = 0.f;
  float powerDeficit = 0.f;
  float charge = 0.f;
};

struct SimResults{
  float TotalTime = 0.f;
  float TimeShareBat = 0.f; // Time Share of Running only on Battery
  float TimeShareEng = 0.f; // Time Share of Running only on Engine
  float TimeShareHybrid = 0.f; // Time Share of Running on Battery and Engine
  float TimeShareLand = 0.f; // Time Share of Charging at Charge Point
  float TimeShareRest = 0.f; // Time Share of (not Running and not charging)
  float EnergyShareLand = 0.f; // Energy Share of Land Connection
  float EnergyShareFuel = 0.f; // Energy Share of Fuel
  float FuelConsumption = 0.f; // KWh of Fuel used
  float InsufPwrT = 0.f; //Insufficient Power Duration in Seconds
  float InsufPwrS1 = 0.f; //Insufficient Power as Time Fraction of Job Time
  float InsufPwrS2 = 0.f; //Insufficient Power as Time Fraction of total Time
  float loadCycles = 0.f; //Battery Load Cycles during simulation

  //CO2
  float CO2Backpack = 0.f; //CO2 Emissions Based on installed Components
  float CO2Operational = 0.f; //CO2 Emission Based on Consumption
  float CO2Total = 0.f; //Sum of CO2Backpack and CO2Operational
  //Cost
  float CostBatteryperLifetime = 0.f;
  float CostFuelperLifeTime = 0.f;
  float CostElectricityperLifeTime = 0.f;
  float Capex = 0.f;
  float Opex = 0.f;
  float TCO = 0.f;

  void display_BS(){
    int width = 15;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Total Time(d)";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Bat Share";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Eng Share";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Hybrid Share";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Land Share";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Idle Share";
    std::cout << std::endl;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TotalTime / secsPerDay;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TimeShareBat / TotalTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TimeShareEng / TotalTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TimeShareHybrid / TotalTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TimeShareLand / TotalTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TimeShareRest / TotalTime;
    std::cout << std::endl << std::endl;
    std::cout << std::left << std::setw(30) << std::setfill(' ') << "Time out of power:" << InsufPwrT << std::endl;
    std::cout << "\n-------------------------------------------------------------------------------\n ########### Predicted performance over assumed lifetime ###########\n";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "TCO";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "OPEX";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "CAPEX";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "CO2";
    std::cout << std::endl;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TCO;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << Opex;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << Capex;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << CO2Total;
    std::cout << "\n-------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Fuel cost";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Electricity";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Load Cycles";
    std::cout << std::endl;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << CostFuelperLifeTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << CostElectricityperLifeTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << loadCycles;
    std::cout << std::endl;
  }

  void display(){
    int width = 15;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Total Time(d)";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Bat Share";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Eng Share";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Hybrid Share";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Land Share";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Idle Share";
    std::cout << std::endl;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TotalTime / secsPerDay;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TimeShareBat / TotalTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TimeShareEng / TotalTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TimeShareHybrid / TotalTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TimeShareLand / TotalTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TimeShareRest / TotalTime;
    std::cout << std::endl << std::endl;
    std::cout << std::left << std::setw(30) << std::setfill(' ') << "Time out of power:" << InsufPwrT << std::endl;
    std::cout << "\n-------------------------------------------------------------------------------\n ########### Predicted performance over assumed lifetime ###########\n";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "TCO";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "OPEX";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "CAPEX";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "CO2";
    std::cout << std::endl;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << TCO;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << Opex;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << Capex;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << CO2Total;
    std::cout << "\n-------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Fuel cost";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Electricity";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Load Cycles";
    std::cout << std::endl;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << CostFuelperLifeTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << CostElectricityperLifeTime;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << loadCycles;
    std::cout << std::endl;
  }
};

struct Battery {
    //General
        std::string Name = "Dummy Battery"; //Name of Battery (should be unique and can therefore be used as identifier if necessary)
        std::string Chemistry = "LFP"; //chemistry type of the Battery Cells. This is important for degradation prognose of Battery Cells (not used so far)

        float ModuleCapacity; /*Battery Systems are made of parallel-connected stacks which consists of modules connected in series.
                                This means Battery-System-Capacity can only be an integer multiple of the Module-Capacity. (not considered so far)*/
        float ModuleVoltage; /*To get the needed system voltage, a number of Modules need to be connected in series.
                                This series is called stack. (not considered so far)
                                E.g.: If the system voltage shall be 600V and a module has a Voltage of 100V, every stack should consist of 6 Modules.
                                In this example the Battery-System can only consist of an integer multible of 6 Modules. */

        int minCap = 0;
        int maxCap = 1000;

        float etaBat = 0.95f; // Battery Efficiency
        float BatteryCO2BackpackperKWH = 123.f; //CO2 emitted in Battery production per kWh Battery Capacity
        float BatteryPricePerKWh = 280.f; // in Euro
        float WeightPerKWh; //maybe interesting in Evaluation
        float VolumePerKWh; //maybe interesting in Evaluation

    //Specification of Lifetime
        float maxCRateChargeCont; // Max. allowed C-Rate (Charge) for long periods
        float maxCRateDischargeCont; // Max. allowed C-Rate (discharge) for long periods
        float maxCRateChargePeak; // Max. allowed C-Rate (Charge) for short peaks
        float maxCRateDischargePeak; // Max. allowed C-Rate (Charge) for short peaks
        float maxPeakLength; // Maximum allowd Peak length in Seconds
        float ChargeLimitSoC = 0.9f; // Maximum allowed SoC
        float DischargeLimitSoC = 0.1f; // Minimum allowd SoC
        int BatCycles = 15000; // Max. Battery cycles till EOL

    //Rack Dimensions (May be usefull for automated volume/space calculation, but not considered so far)
        float MaxModulesPerRack; //Batteries usually come in rack-housings simular to server-racks. This value indicates the maximum number of modules that fit into the standard rack of the maker.
        float RackHeight; // in mm
        float RackWidth; // in mm
        float Rackdepth; // in mm

        void info() const {
          int width = 15;
          std::cout << std::left << std::setw(2*width) << std::setfill(' ') << "Name";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Min Capacity";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Max Capacity";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Efficiency";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Price/kWh";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "CO2/kWh";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Charge Limit";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Discharge Limit";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Max Cycles" << std::endl;

          std::cout << std::left << std::setw(2*width) << std::setfill(' ') << Name;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << minCap;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << maxCap;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << etaBat;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << BatteryPricePerKWh;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << BatteryCO2BackpackperKWH;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << ChargeLimitSoC;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << DischargeLimitSoC;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << BatCycles << std::endl;
        }
};

struct EnergyConverter {
    //General
        std::string Name = "Default Engine"; //Name of Energy Converter (should be unique and can therefore be used as identifier if necessary)
        std::string Fuel = "Various Hydrocarbons"; //Name of Fuel

    //Optimization constraints
        int minCount = 0;
        int maxCount = 2;

    //Efficiency
        /*The efficiency of energy converters depends on the load point.
        Unfortunately these efficiency-curves are hard to get from the manufacturer.
         Therefore we only define single load points and may interpolate in between. (so far "RangeEx" needs just a single Value as Efficiency this needs to be changed)*/
        float eff25 = 0.8f; //Efficiency at 25% Load
        float eff50 = 0.8f; //Efficiency at 50% Load
        float eff75 = 0.8f; //Efficiency at 75% Load
        float eff85 = 0.8f; //Efficiency at 85% Load (This Data Point does not fit into the grid, but most Engines have their best efficiency in this area and most engine Manufacturers publish values for this load point)
        float eff100 = 0.8f; //Efficiency at 100% Load
        float Setpoint = 1.f; // Loadpoint for max Efficiency (in %)

    //Power
        float PwrSet = 0.85f; // Engine Power at Setpoint
        float PwrMax = 415.f; // Maximum Engine Power

    //CO2
        float FuelC02EmissionsperKWH = .25f; // kg per kWh
        float EnergyConverterCO2Backpack = 2600.f; // kg CO2

    //Cost
        float FuelPriceperKWH = 0.266f; // Euro per kWh
        float EnergyConverterPrice = 100000.f; // Euro

    //Dimensions and Weight (May be usefull for automated volume/space calculation, but not considered so far)
        float Weight;
        float Width;
        float Height;
        float Depth;
        float FuelEnergyDensity; //kg per kWh (this can be used to calculate Fuel Tank volumes and refueling intervals; not considered so far)

        float efficiency(int pwr){
          float pctLoad = (float) pwr / PwrMax;
          if(pctLoad < 0.25) return eff25;
          if(pctLoad < 0.50) return eff50;
          if(pctLoad < 0.75) return eff75;
          if(pctLoad < 0.85) return eff85;
          return eff100;
        }

        void info() const {
          int width = 15;
          std::cout << std::left << std::setw(2*width) << std::setfill(' ') << "Name";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Max Power";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Setpoint";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Price/kWh";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "CO2/kWh";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "CO2 Backpack";
          std::cout << std::left << std::setw(width) << std::setfill(' ') << "Capex" << std::endl;

          std::cout << std::left << std::setw(2*width) << std::setfill(' ') << Name;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << PwrMax;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << Setpoint;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << FuelPriceperKWH;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << FuelC02EmissionsperKWH;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << EnergyConverterCO2Backpack;
          std::cout << std::left << std::setw(width) << std::setfill(' ') << EnergyConverterPrice << std::endl;
        }
};

struct ChargePoint{
  std::string Name = "Default ChargePoint";
  int pluginDelay = 120;
  int powerLimit = 75;
  float electricityPriceperKWh = 0.08f;
  float electricityCO2EmissionperKWH = 10.f;
  int Capex = 50000;
  geoFence gf = {53.539f, 53.541f, 9.878f, 9.882f};

  void info() const {
    int width = 15;
    std::cout << std::left << std::setw(2*width) << std::setfill(' ') << "Name";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Delay";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Power Limit";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Price/kWh";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "CO2/kWh";
    std::cout << std::left << std::setw(width) << std::setfill(' ') << "Capex" << std::endl;

    std::cout << std::left << std::setw(2*width) << std::setfill(' ') << Name;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << pluginDelay;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << powerLimit;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << electricityPriceperKWh;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << electricityCO2EmissionperKWH;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << Capex << std::endl;
  }
};

struct Ship{
  float targetLifetime = 20.f;
  float BatteryCap = 400.f;
  int installedEngines = 1;

  Record state;

  std::shared_ptr<Battery> battery;
  std::shared_ptr<EnergyConverter> engine;
  std::shared_ptr<controlStrategy> ctrl;
  std::shared_ptr<ChargePoint> chargePoint;

  void info() const;

};

std::vector<std::shared_ptr<EnergyConverter>> enginesFromFile(const std::string fname);
std::vector<std::shared_ptr<Battery>> batteriesFromFile(const std::string fname);
std::vector<std::shared_ptr<ChargePoint>> chargePointsFromFile(const std::string fname);

#endif
