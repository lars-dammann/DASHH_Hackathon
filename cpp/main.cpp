#include "dataSource.hpp"
#include "components.hpp"
#include "performanceModel.hpp"
#include "csvReader.hpp"
#include "simulator.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <string>

class Timer
{
private:
    std::chrono::time_point<std::chrono::steady_clock> start;
    std::string name;
public:
    Timer(std::string&& _name) : name{_name}{start = std::chrono::steady_clock::now();}
    ~Timer(){
      std::chrono::duration<float> t = std::chrono::steady_clock::now() - start;
      std::cout << name << " took " << t.count() << " seconds\n";
    }
};

void recordingToCSV(std::vector<Record>& dat, std::string fname){
  std::ofstream f(fname, std::ios::out);
  if (f.is_open()) {
    for(auto& r : dat)
      f << r.time << ", " << r.load << ", " << r.engPwr << ", " << r.batPwr << ", " << r.charge << "\n";
    f.close();
  }
}

void print(std::vector<std::pair<Ship, SimResults>>& lst){

  int width = 15;
  std::cout << std::left << std::setw(6) << std::setfill(' ') << "-OOP-";
  std::cout << std::left << std::setw(2*width) << std::setfill(' ') << "Battery";
  std::cout << std::left << std::setw(2*width) << std::setfill(' ') << "Engine";
  std::cout << std::left << std::setw(2*width) << std::setfill(' ') << "Charge Point";
  std::cout << std::left << std::setw(width) << std::setfill(' ') << "Battery Cap";
  std::cout << std::left << std::setw(width) << std::setfill(' ') << "# Engines";
  std::cout << std::left << std::setw(width) << std::setfill(' ') << "TCO";
  std::cout << std::left << std::setw(width) << std::setfill(' ') << "Ctrl params";
  std::cout << std::endl;
  for(auto& [s, res] : lst){
    std::cout << std::left << std::setw(6) << std::setfill(' ') << res.InsufPwrT;
    std::cout << std::left << std::setw(2*width) << std::setfill(' ') << s.battery->Name;
    std::cout << std::left << std::setw(2*width) << std::setfill(' ') << s.engine->Name;
    std::cout << std::left << std::setw(2*width) << std::setfill(' ') << s.chargePoint->Name;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << s.BatteryCap;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << s.installedEngines;
    std::cout << std::left << std::setw(width) << std::setfill(' ') << res.TCO;
    std::cout << "  ( ";
    for(auto i : s.ctrl->getParamList()) std::cout << i << "  ";
    std::cout << ")" << std::endl;
  }
}



int main(int argc, char const *argv[]) {

  std::string fname = "Lotse1-21_01_01-21_01_09.csv";
  FileSource fs(fname,  {53.539,53.541,9.878,9.882});

  //Test creation of default Markov chain container
  MarkovSource M(5,3);
  M.matchToData(fs);

  //M.printModelInfo();

  std::ofstream myfile;
  std::string outname;
  int counter;
  int SecsPerWeek = 7*24*60*60;

  for(int i = 0; i < 30; i++){
    M.sample(10);
    outname = "../data/dataset" + std::to_string(i) + ".csv";
    std::cout << outname;
    myfile.open(outname);
    counter = 0;
    myfile << "time,load,charge\n";
    for(auto[load, charge] : M){
        myfile << 10*i << "," << load << "," << charge << "\n";
        counter++;
        if(counter > SecsPerWeek/10) break;
      }
    myfile.close();
  }



  /*
  auto re = std::make_shared<rangeExtender>();
  auto dummyEngine = std::make_shared<EnergyConverter>();
  auto dummyBattery = std::make_shared<Battery>();
  auto dummyCP = std::make_shared<ChargePoint>();

  Ship testShip;
  testShip.ctrl = re;
  testShip.engine = dummyEngine;
  testShip.battery = dummyBattery;
  testShip.chargePoint = dummyCP;

  testShip.info();
  testShip.battery->info();
  testShip.engine->info();
  testShip.chargePoint->info();

  auto[results, recording] = simulate(testShip,fs);
  results.display();

  recordingToCSV(recording, "temp.csv");
  */
  return 0;
}
