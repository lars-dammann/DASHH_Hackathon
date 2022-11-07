#include "components.hpp"
#include <vector>
#include <memory>
#include <fstream>

std::vector<std::shared_ptr<ChargePoint>> chargePointsFromFile(const std::string fname){
  std::vector<std::shared_ptr<ChargePoint>> rval;
  std::ifstream f(fname,  std::ios::in);
  if (f.is_open()) {
    std::string line;
    std::getline(f,line); // Read header

    while (std::getline(f, line)) {
      std::vector<std::string> tokens;
      std::string sep = "|";
      size_t pos = 0;
      //Split CSV with | as separator
      while((pos = line.find(sep))!= std::string::npos ){
        tokens.push_back(line.substr(0, pos));
        line.erase(0,pos+sep.length());
      }
      tokens.push_back(line);
      auto cp = std::make_shared<ChargePoint>();
      cp->Name = tokens[0];
      cp->pluginDelay = std::stoi(tokens[1]);
      cp->powerLimit = std::stoi(tokens[2]);
      cp->electricityPriceperKWh = std::stof(tokens[3]);
      cp->electricityCO2EmissionperKWH = std::stof(tokens[4]);
      cp->Capex = std::stoi(tokens[5]);
      cp->gf = {std::stof(tokens[6]), std::stof(tokens[7]), std::stof(tokens[8]),std::stof(tokens[9])};
      rval.push_back(cp);
    }
    f.close();
  }
  else {
    std::cerr << "Unable to open file\n";
  }
  return rval;
}

std::vector<std::shared_ptr<Battery>> batteriesFromFile(const std::string fname){
  std::vector<std::shared_ptr<Battery>> rval;
  std::ifstream f(fname,  std::ios::in);
  if (f.is_open()) {
    std::string line;
    std::getline(f,line); // Read header

    while (std::getline(f, line)) {
      std::vector<std::string> tokens;
      std::string sep = "|";
      size_t pos = 0;
      //Split CSV with | as separator
      while((pos = line.find(sep))!= std::string::npos ){
        tokens.push_back(line.substr(0, pos));
        line.erase(0,pos+sep.length());
      }
      tokens.push_back(line);
      auto bat = std::make_shared<Battery>();
      bat->Name = tokens[0];

      bat->minCap = std::stoi(tokens[1]);
      bat->maxCap = std::stoi(tokens[2]);

      bat->ModuleCapacity = std::stof(tokens[3]);
      bat->ModuleVoltage  = std::stof(tokens[4]);

      bat->MaxModulesPerRack = std::stof(tokens[5]);
      bat->RackHeight = std::stof(tokens[6]);
      bat->RackWidth = std::stof(tokens[7]);
      bat->Rackdepth = std::stof(tokens[8]);

      bat->maxCRateChargeCont = std::stof(tokens[9]);
      bat->maxCRateDischargeCont = std::stof(tokens[10]);
      bat->maxCRateChargePeak = std::stof(tokens[11]);
      bat->maxCRateDischargePeak = std::stof(tokens[12]);
      bat->maxPeakLength = std::stof(tokens[13]);

      bat->DischargeLimitSoC = std::stof(tokens[14]);
      bat->ChargeLimitSoC = std::stof(tokens[15]);

      bat->BatCycles = std::stoi(tokens[16]);

      bat->etaBat = std::stof(tokens[17]);
      bat->BatteryPricePerKWh = std::stof(tokens[18]);
      bat->BatteryCO2BackpackperKWH = std::stof(tokens[19]);

      bat->WeightPerKWh = std::stof(tokens[20]);
      bat->VolumePerKWh = std::stof(tokens[21]);

      rval.push_back(bat);
    }
    f.close();
  }
  else {
    std::cerr << "Unable to open file\n";
  }
  return rval;
}

std::vector<std::shared_ptr<EnergyConverter>> enginesFromFile(const std::string fname){
  std::vector<std::shared_ptr<EnergyConverter>> rval;
  std::ifstream f(fname,  std::ios::in);
  if (f.is_open()) {
    std::string line;
    std::getline(f,line); // Read header

    while (std::getline(f, line)) {
      std::vector<std::string> tokens;
      std::string sep = "|";
      size_t pos = 0;
      //Split CSV with | as separator
      while((pos = line.find(sep))!= std::string::npos ){
        tokens.push_back(line.substr(0, pos));
        line.erase(0,pos+sep.length());
      }
      tokens.push_back(line);
      auto eng = std::make_shared<EnergyConverter>();
      eng->Name = tokens[0];

      eng->minCount = std::stoi(tokens[1]);
      eng->maxCount = std::stoi(tokens[2]);
      eng->PwrMax = std::stof(tokens[3]);

      eng->Setpoint = std::stof(tokens[4]);
      eng->eff25 = std::stof(tokens[5]) / 100.f;
      eng->eff50 = std::stof(tokens[6]) / 100.f;
      eng->eff75 = std::stof(tokens[7]) / 100.f;
      eng->eff85 = std::stof(tokens[8]) / 100.f;
      eng->eff100 = std::stof(tokens[9]) / 100.f;

      eng->EnergyConverterPrice = std::stof(tokens[10]);
      eng->FuelPriceperKWH = std::stof(tokens[11]);
      eng->FuelC02EmissionsperKWH = std::stof(tokens[12]);
      eng->EnergyConverterCO2Backpack = std::stof(tokens[13]);

      eng->Weight = std::stof(tokens[13]);
      eng->Depth = std::stof(tokens[14]);
      eng->Width = std::stof(tokens[15]);
      eng->Height = std::stof(tokens[16]);



      rval.push_back(eng);
    }
    f.close();
  }
  else {
    std::cerr << "Unable to open file\n";
  }
  return rval;
}

void Ship::info() const{
  std::cout << "Installed " << installedEngines << " Engines of type " << engine->Name << "\n";
  std::cout << "Installed " << BatteryCap << " kWhs of type " << battery->Name << "\n";
  std::cout << "Ctrl params: ";
  for(auto i : ctrl->getParamList()) std::cout << i << "  ";
  std::cout << std::endl;
}
