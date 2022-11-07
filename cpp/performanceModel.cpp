#include "performanceModel.hpp"
#include "event.hpp"
#include "dataSource.hpp"
#include <numeric>
#include <cmath>

constexpr int MARKOV_RESOLUTION = 10;
constexpr int MAX_ITERATIONS_REFINEMENT = 50;

performanceModel::performanceModel(int numProfiles, int timeResolution) : numProfiles{numProfiles}, timeResolution{timeResolution}{
  if(numProfiles < 2) {
    std::cerr << "Requested performanceModel with " << numProfiles << " subprofiles, setting profile count to 2" << std::endl;
    numProfiles = 2;
  }
  transitionMatrix = std::vector<std::vector<float>>(timeResolution * numProfiles, std::vector<float>(timeResolution * numProfiles, 0.));
  profiles = std::vector<Profile*>(timeResolution * numProfiles, nullptr);
  for(auto& p : profiles){
    p = new Profile;
  }
  for(int i = 0; i < timeResolution * numProfiles; i++){
    transitionMatrix[i][i] = 1.;
  }
  activeProfile = 0;
}

performanceModel::performanceModel() {
  numProfiles = 0;
  transitionMatrix = {};
  profiles = {};
}

performanceModel::~performanceModel(){
  for(auto& p : profiles) delete p;
}

void performanceModel::transition(){
  double rnd = rng01();
  for(int i = 0; i < timeResolution * numProfiles; i++){
    rnd -= transitionMatrix[activeProfile][i];
    if(rnd < 0){
      activeProfile = i;
      break;
    }
  }
}

int performanceModel::sample(int timeStep){
  double draw = 0.;

  auto p = profiles[activeProfile];
  if(!p->isActive()){
    transition();
     p = profiles[activeProfile];
     p->activate();
  }
  draw += p->sample(timeStep);
  if( isCharging() ) draw = 0.;
  return (int) std::round(draw);
}

void performanceModel::info(){
  for( const auto& p : profiles) p->printInfo();
  for( const auto& row : transitionMatrix){
    for(const auto x : row) std::cout << x << " ";
    std::cout << std::endl;
  }
}

bool isCharging(std::vector<chargeTime>& chargingTimes, int time){return true;};

double refine(std::vector<EngineEvent>& loads, std::vector<int>& centers, std::vector<chargeTime>& chargingTimes){
  long totalError = 0;
  int k = (int) centers.size();
  std::vector<long> accumulateClusters(k);
  std::vector<long> clusterSizes(k);
  int minTime = loads[0].time;
  int maxTime = loads[loads.size()-1].time;
  int count = 0;
  for(int time = minTime; time < maxTime; time+=MARKOV_RESOLUTION){
    if(isCharging(chargingTimes, time)) continue;
    count++;
    int load = getCurLoad(&loads, time);
    int closest = 0;
    for(int i = 0; i < k; i++){
      if(std::abs(load - centers[i]) < std::abs(load - centers[closest]) ) closest = i;
    }
    accumulateClusters[closest] += load;
    clusterSizes[closest]++;
    totalError += std::abs(load - centers[closest]);
  }
  for(int i = 0; i < k; i++){
    centers[i] = (int) std::round((double) accumulateClusters[i] / clusterSizes[i]);
  }
  return (double) totalError / count;
}
std::vector<int> kCluster(std::vector<EngineEvent>& loads, int k, std::vector<chargeTime>& chargingTimes){
  std::vector<int> rval(k);
  int cum = 50;
  for(auto&i : rval){
    cum += 50;
    i = cum;
  }
  double error = 1e12;
  int rounds = 0;
  while(true){
    rounds++;
    double newError = refine(loads, rval, chargingTimes);
    if((error-newError) < 0.1 || rounds > 15) break;
    error = newError;
  }
  std::cout << "Used " << rounds << " rounds of clustering to match data, avg error " << error << std::endl;
  return rval;
}
int getPhaseID(int time, int load, std::vector<int>& centers, std::vector<chargeTime>& chargingTimes){

  if(isCharging(chargingTimes, time)) return (int) centers.size();
  int rval = 0;
  for(int i = 0; i < centers.size(); i++){
    if(std::abs(load - centers[rval]) > std::abs(load - centers[i]) ) rval = i;
  }
  return rval;
}
void cleanupPartition(std::vector<MarkovPhase>& partition){
  int i = 1;
  while(i < partition.size()-1){
    auto& cur = partition[i];
    auto& prev = partition[i-1];
    auto& next = partition[i+1];
    if(cur.endTime - cur.startTime <= 60 && prev.id == next.id){
      prev.endTime = next.endTime;
      partition.erase(partition.begin()+i, partition.begin()+i+2);
      i--;
    }
    i++;
  }
  auto n = partition.size();
  if(partition[0].id == partition[n-1].id){
    partition[0].startTime -= (partition[n-1].endTime - partition[n-1].startTime);
    partition.erase(partition.begin() + n-1);
  }
}
std::vector<MarkovPhase> partition(std::vector<EngineEvent>& loads, std::vector<chargeTime>& chargingTimes, std::vector<int>& centers, int k){
  std::vector<MarkovPhase> rval;
  int start = (*loads.begin()).time;
  int end = (*(loads.end()-1)).time;
  int curTime = start;
  int curPhase = getPhaseID(curTime, getCurLoad(&loads, curTime), centers, chargingTimes);
  int phaseStart = start;
  while(curTime < end){
    curTime += MARKOV_RESOLUTION;
    if(getPhaseID(curTime, getCurLoad(&loads, curTime), centers, chargingTimes) == curPhase) continue;
    rval.push_back({phaseStart, curTime, curPhase});
    curPhase = getPhaseID(curTime, getCurLoad(&loads, curTime), centers, chargingTimes);
    phaseStart = curTime;
  }
  rval.push_back({phaseStart, curTime, curPhase});
  return rval;
}

void refinePartitionByTime(std::vector<MarkovPhase>& partition, int numProfiles, int k){

  std::vector< std::vector<int> > centers(numProfiles, std::vector<int>(k, 0));

  for(auto prt : partition){
    for(int i = 0; i < k; i++){

        centers[prt.id][i] = prt.endTime-prt.startTime + k;
    }
  }

  int rounds = 0;
  float error = 15000.;

  while(rounds < 20 && error > 30){

    rounds++;
    std::vector< std::vector<int> > newCenters(numProfiles, std::vector<int>(k, 0));
    std::vector< std::vector<int> > count(numProfiles, std::vector<int>(k, 0));
    float newError = 0;
    for(const auto& phase : partition){
      auto duration = phase.endTime - phase.startTime;
      int closest = 0;
      for(int i = 0; i < k; i++){
        if(std::abs(centers[phase.id][i] - duration) < std::abs(centers[phase.id][closest] - duration) ) closest = i;
      }
      newError += std::abs(duration - centers[phase.id][closest]);
      newCenters[phase.id][closest] += duration;
      count[phase.id][closest]++;
    }

    for(int i = 0; i < numProfiles; i++){
      for(int j = 0; j < k; j++){
        if(count[i][j] == 0) {
          newCenters[i][j] = 500;
        }else{
          newCenters[i][j] = newCenters[i][j] / count[i][j];
        }
      }
    }
    centers = std::move(newCenters);
    newError = newError / partition.size();
    if((error - newError) < 2) break;
    error = newError;
  }

  for(auto& prt : partition){
      auto duration = prt.endTime - prt.startTime;
      int closest = 0;
      for(int i = 0; i < k; i++){
        if( std::abs(duration - centers[prt.id][i]) <= std::abs(duration - centers[prt.id][closest]) )
          closest = i;
      }
      prt.id = prt.id * k + closest;
  }
}

void performanceModel::fitTransitionMatrix(std::vector<MarkovPhase>& phases){
  for(auto& row : transitionMatrix){
    for(auto& x : row)  x = 0;
    }
  for(int i = 0; i < phases.size()-1; i++){
    transitionMatrix[phases[i].id][phases[i+1].id]++;
  }
  for(auto& row : transitionMatrix){
    float sum = 0;
    for(auto x : row) sum += x;
    if(sum == 0)
        std::cerr << "Empty row in transition Matrix" << std::endl;
    else
        for(auto& x : row) x /= sum;
  }
}

void performanceModel::matchToData(std::vector<EngineEvent>& loads, std::vector<chargeTime>& chargingTimes, bool dbg){
  //Cluster by loads into numProfiles - 1 parts. The final part is reserved for charging behaviour

  auto centers = kCluster(loads, numProfiles-1, chargingTimes);
  std::cout << "Clustered by load" << std::endl;
  /*
  //Break input into discrete phases acording to load
  std::vector<MarkovPhase> prt = partition(loads, chargingTimes, centers, numProfiles);
  //Split each load group into timeResolution many subparts
  refinePartitionByTime(prt, numProfiles, timeResolution);
  //cleanupPartition(prt);
  std::cout << "Refined clustering by time" << std::endl;
  //Get avg load and standard deviation of load for each cluster
  fitTransitionMatrix(prt);

  std::vector<double> loadVar2(timeResolution * numProfiles, 0);
  std::vector<int> loadCount(timeResolution * numProfiles, 0);

  std::vector<double> totalTimes(timeResolution * numProfiles, 0);
  std::vector<int> timesCount(timeResolution * numProfiles, 0);
  std::vector<double> timeVar2(timeResolution * numProfiles, 0);

  //Compute average durations per ID
  for(const auto& p : prt){
    auto duration = p.endTime - p.startTime;
    timesCount[p.id]++;
    totalTimes[p.id] += duration;
  }
  for(int i = 0; i < timeResolution * numProfiles; i++)  if(timesCount[i]) totalTimes[i] /= timesCount[i];

  //Compute standard deviations per ID
  for(auto& p : prt){
    timeVar2[p.id] += (p.endTime - p.startTime - totalTimes[p.id]) * (p.endTime - p.startTime - totalTimes[p.id]);
    for(int t = p.startTime; t < p.endTime; t += MARKOV_RESOLUTION){
      loadCount[p.id]++;
      auto load = getCurLoad(&loads, t);
      if( (p.id / timeResolution) < numProfiles-1 )loadVar2[p.id] += (centers[p.id / timeResolution] - load) * (centers[p.id / timeResolution] - load);
    }
  }

  for(int i = 0; i < timeResolution * numProfiles; i++){
    timeVar2[i] = std::sqrt(timeVar2[i] / timesCount[i]);
    loadVar2[i] = std::sqrt(loadVar2[i] / loadCount[i]);
  }



  for(int id = 0; id < timeResolution * numProfiles; id++){
    if( (id / timeResolution) >= numProfiles-1 ){
      profiles[id]->avgPwr = -10; // Charging Profiles
      profiles[id]->stdPwr = 0;
      profiles[id]->avgTme = totalTimes[id];
      profiles[id]->stdTme = timeVar2[id];
    }else{
      profiles[id]->avgPwr = centers[id / timeResolution]; //Non-charging Profiles
      profiles[id]->stdPwr = loadVar2[id];
      profiles[id]->avgTme = totalTimes[id];
      profiles[id]->stdTme = timeVar2[id];
    }
  }

  for(auto p : profiles) p->initDistributions();
  if(dbg){
    for(auto p : profiles) p->printInfo();
    std::cout << "Transition Matrix: " << std::endl;
    for(auto& row : transitionMatrix){
      for(auto x : row) std::cout << "  " << x;
    std::cout << std::endl;
    }
  }
  */
}

constexpr int MAXLOAD = 10000;

//centers should be sorted, histogram is implicitly a (load -> count) map
void refine(std::vector<int>& centers, const std::vector<int>& histogram ){
  std::vector<int> newSum(centers.size(), 0);
  std::vector<int> newCount(centers.size(), 0);
  int curLoad = 0;
  int curCenter = 0;
  while(curCenter < centers.size() && curLoad < histogram.size()){
    if( curCenter == centers.size()-1 || std::abs(centers[curCenter] - curLoad) <= std::abs(centers[curCenter +1] -curLoad) ){
      newCount[curCenter] += histogram[curLoad];
      newSum[curCenter] += histogram[curLoad] * curLoad;
      curLoad++;
    }else{
      curCenter++;
    }
  }

  for(int i = 0; i < centers.size(); i++){
    centers[i] = newSum[i] / newCount[i];
  }

}

void performanceModel::matchToData(FileSource& fs){
    std::vector<int> loadCenters(numProfiles-1);
    std::vector<int> loadHistogram(MAXLOAD, 0); //collect data into histogram for speedup

    //get loads into histogram
    for(auto[load, canCharge] : fs){
      if(canCharge) continue;
      loadHistogram[(int) std::floor(load)]++;
    }

    //compute max load seen
    int maxLoad = 0;
    for(int i = 0; i < MAXLOAD; i++){
      if (loadHistogram[i] > 0) maxLoad = i;
    }
    loadHistogram.resize(maxLoad+1);

    //partition interval to get intial centers
    for(int i = 0; i < numProfiles-1; i++){
      loadCenters[i] =  i * (maxLoad / (numProfiles-1));
    }

    for(int i = 0; i < MAX_ITERATIONS_REFINEMENT; i++){refine(loadCenters, loadHistogram);}


    std::vector<int> deviations(numProfiles-1, 0);
    std::vector<int> devCount(numProfiles-1, 0);

    int curLoad = 0;
    int curCenter = 0;
    while(curCenter < loadCenters.size() && curLoad < loadHistogram.size()){
      if( curCenter == loadCenters.size()-1 || std::abs(loadCenters[curCenter] - curLoad) <= std::abs(loadCenters[curCenter +1] -curLoad) ){
        deviations[curCenter] += loadHistogram[curLoad] * (curLoad - loadCenters[curCenter]) * (curLoad - loadCenters[curCenter]);
        devCount[curCenter] += loadHistogram[curLoad];
        curLoad++;
      }else{
        curCenter++;
      }
    }

    for(int i = 0; i < numProfiles-1; i++){
      for(int j = 0; j < timeResolution; j++){
        profiles[i*timeResolution + j]->avgPwr = loadCenters[i];
        profiles[i*timeResolution + j]->stdPwr = std::sqrt(deviations[i] / devCount[i]);
      }
    }


    //Extract phases to cluster by time
    int res = fs.getResolution();
    std::vector<MarkovPhase> phases;
    int curTime = 0;
    int curID = -1;
    for(auto[load, canCharge] : fs){
      auto closestCenter = std::min_element(loadCenters.begin(), loadCenters.end(), [load](int first, int second){return std::abs(load-first) < std::abs(load-second);});
      int id = std::distance(loadCenters.begin(), closestCenter);
      if(canCharge) id = numProfiles-1; // fixed charging profile id

      if (id == curID) {
          (*(phases.end() - 1)).endTime += res;
      }else{
        phases.push_back({curTime-res, curTime, id});
      }
      curID = id;
      curTime += res;
    }
    cleanupPartition(phases);
    refinePartitionByTime(phases, numProfiles, timeResolution);
    fitTransitionMatrix(phases);
    for(int id = 0; id < numProfiles * timeResolution; id++){
      std::vector<int> times;
      for(auto p : phases) if(p.id == id) times.push_back(p.endTime-p.startTime);
      int meanTime = 0;
      float stddev = 0;
      if (times.size() > 0) {
          meanTime = std::accumulate(times.begin(), times.end(), 0) / times.size();
          stddev = std::accumulate(times.begin(), times.end(), 0.f, [meanTime](float acc, int item) {return (item - meanTime) * (item - meanTime); }) / times.size();
      }
      int deviation = (int) std::round(std::sqrt(stddev));
      profiles[id]->avgTme = meanTime;
      profiles[id]->stdTme = deviation;
    }
    for(auto& p : profiles) p->initDistributions();
}

void performanceModel::matchToData(GenericSource& gs){
    std::vector<int> loadCenters(numProfiles-1);
    std::vector<int> loadHistogram(MAXLOAD, 0); //collect data into histogram for speedup

    //get loads into histogram
    for(auto[load, canCharge] : gs){
      if(canCharge) continue;
      loadHistogram[(int) std::floor(load)]++;
    }

    //compute max load seen
    int maxLoad = 0;
    for(int i = 0; i < MAXLOAD; i++){
      if (loadHistogram[i] > 0) maxLoad = i;
    }
    loadHistogram.resize(maxLoad+1);

    //partition interval to get intial centers
    for(int i = 0; i < numProfiles-1; i++){
      loadCenters[i] =  i * (maxLoad / (numProfiles-1));
    }

    for(int i = 0; i < MAX_ITERATIONS_REFINEMENT; i++){refine(loadCenters, loadHistogram);}


    std::vector<int> deviations(numProfiles-1, 0);
    std::vector<int> devCount(numProfiles-1, 0);

    int curLoad = 0;
    int curCenter = 0;
    while(curCenter < loadCenters.size() && curLoad < loadHistogram.size()){
      if( curCenter == loadCenters.size()-1 || std::abs(loadCenters[curCenter] - curLoad) <= std::abs(loadCenters[curCenter +1] -curLoad) ){
        deviations[curCenter] += loadHistogram[curLoad] * (curLoad - loadCenters[curCenter]) * (curLoad - loadCenters[curCenter]);
        devCount[curCenter] += loadHistogram[curLoad];
        curLoad++;
      }else{
        curCenter++;
      }
    }

    for(int i = 0; i < numProfiles-1; i++){
      for(int j = 0; j < timeResolution; j++){
        profiles[i*timeResolution + j]->avgPwr = loadCenters[i];
        profiles[i*timeResolution + j]->stdPwr = std::sqrt(deviations[i] / devCount[i]);
      }
    }


    //Extract phases to cluster by time
    int res = gs.getResolution();
    std::vector<MarkovPhase> phases;
    int curTime = 0;
    int curID = -1;
    for(auto[load, canCharge] : gs){
      auto closestCenter = std::min_element(loadCenters.begin(), loadCenters.end(), [load](int first, int second){return std::abs(load-first) < std::abs(load-second);});
      int id = std::distance(loadCenters.begin(), closestCenter);
      if(canCharge) id = numProfiles-1; // fixed charging profile id

      if (id == curID) {
          (*(phases.end() - 1)).endTime += res;
      }else{
        phases.push_back({curTime-res, curTime, id});
      }
      curID = id;
      curTime += res;
    }
    cleanupPartition(phases);
    refinePartitionByTime(phases, numProfiles, timeResolution);
    fitTransitionMatrix(phases);
    for(int id = 0; id < numProfiles * timeResolution; id++){
      std::vector<int> times;
      for(auto p : phases) if(p.id == id) times.push_back(p.endTime-p.startTime);
      int meanTime = 0;
      float stddev = 0;
      if (times.size() > 0) {
          meanTime = std::accumulate(times.begin(), times.end(), 0) / times.size();
          stddev = std::accumulate(times.begin(), times.end(), 0.f, [meanTime](float acc, int item) {return (item - meanTime) * (item - meanTime); }) / times.size();
      }
      int deviation = (int) std::round(std::sqrt(stddev));
      profiles[id]->avgTme = meanTime;
      profiles[id]->stdTme = deviation;
    }
    for(auto& p : profiles) p->initDistributions();
}
