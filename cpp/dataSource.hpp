#ifndef DATA_SOURCE_HPP
#define DATA_SOURCE_HPP


#include <memory>
#include <string>
#include "performanceModel.hpp"


class DataSource {
public:
  virtual float getLoad(int timePoint){return 0.;}
  virtual bool canCharge(int timePoint){return false;}
  virtual int getResolution(){return 1;}
  DataSource() = default;
  virtual ~DataSource () = default;
};

class MarkovSource : public DataSource {
private:
  int maxTime = 31536000;
  int res = 10;
  std::unique_ptr<performanceModel> Model;

public:
  struct Iterator{

    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = std::pair<int,bool>; // Data contained in source is (load, canCharge)
    using pointer           = int;  //timePoint only, bit weird
    using reference         = value_type&;


    reference operator*() {if(cache) return cachedVal; cachedVal = parent->sample(res); cache = true; return cachedVal; }
    //pointer operator->() {return curTime;}

    Iterator& operator++(){if(!cache) parent->sample(res); cache = false; curTime+=res; return *this;}
    Iterator& operator++(int){Iterator rval = *this; ++(*this); return rval;}

    Iterator(MarkovSource* parent, int res, pointer curTime) : parent(parent), res(res), curTime(curTime){}

    friend bool operator==(const Iterator& l, const Iterator& r){return l.curTime >= r.curTime;}
    friend bool operator!=(const Iterator& l, const Iterator& r){return l.curTime != r.curTime;}

  private:
    MarkovSource* parent;
    pointer curTime;
    int res;
    value_type cachedVal;
    bool cache = false;
  };

  Iterator begin(){return Iterator(this, res, 0);}
  Iterator end()  {return Iterator(this, res, maxTime);}

  std::pair<int, bool> sample(int res){int load = Model->sample(res); bool charging = Model->isCharging(); return {load, charging};}

  MarkovSource(){Model = std::make_unique<performanceModel>();}
  MarkovSource(int numProfiles, int timeResolution){Model = std::make_unique<performanceModel>(numProfiles, timeResolution);}

  void printModelInfo(){Model->info();}
  void matchToData(FileSource& fs){Model->matchToData(fs);}
  void matchToData(GenericSource& gs){Model->matchToData(gs);}
  //void fitToData(std::vector<EngineEvent>& loads, std::vector<chargeTime>& chargingTimes){Model->matchToData(loads, chargingTimes);}

  float getLoad(int timePoint)  {return 0;}  //Not to be used, since source is dynamic
  bool canCharge(int timePoint) {return false;} //Not to be used, since source is dynamic
  int getResolution()           {return res;}
  ~MarkovSource(){}
};

class GenericSource : public DataSource{
private:

  int sampleResolution = 1;

  struct dataPoint{
    bool canCharge;
    float load;
  };
  std::vector<dataPoint> data = {};
public:

  struct Iterator{

    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = std::pair<float,bool>;
    using pointer           = int;
    using reference         = value_type;


    reference operator*() const {return std::make_pair<float,bool>(parent->getLoad(index * parent->getResolution()) , parent->canCharge(index * parent->getResolution()));}
    //pointer operator->() {return curTime;}

    Iterator& operator++(){index++; return *this;}
    Iterator& operator++(int){Iterator rval = *this; ++(*this); return rval;}

    Iterator(GenericSource* parent, int indx) : parent(parent), index(indx){}

    friend bool operator==(const Iterator& l, const Iterator& r){return l.index == r.index;}
    friend bool operator!=(const Iterator& l, const Iterator& r){return l.index != r.index;}
  private:
    GenericSource* parent;
    pointer index;

  };

  Iterator begin(){return Iterator(this, 0); }
  Iterator end(){return Iterator(this, data.size());}
  float getLoad(int timePoint){if(timePoint / sampleResolution >= data.size()) return 0; return data[timePoint / sampleResolution].load;}
  bool canCharge(int timePoint){if(timePoint / sampleResolution >= data.size()) return false; return data[timePoint / sampleResolution].canCharge;}
  int getResolution(){return sampleResolution;}
  void addPoint(float load, bool canCharge){data.push_back({canCharge, load});}
  void discardData(){data = {};}
  GenericSource() = default;

};

class FileSource : public DataSource{
private:
  geoFence gf;
  int sampleResolution = 10;

  struct dataPoint{
    int time;
    float SOG;
    float Lat;
    float Lon;
    float load;
  };
  std::vector<dataPoint> data = {};
public:

  struct Iterator{

    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = std::pair<float,bool>;
    using pointer           = int;
    using reference         = value_type;


    reference operator*() const {return std::make_pair<float,bool>(parent->getLoad(curTime), parent->canCharge(curTime));}
    //pointer operator->() {return curTime;}

    Iterator& operator++(){curTime+=res; return *this;}
    Iterator& operator++(int){Iterator rval = *this; ++(*this); return rval;}

    Iterator(FileSource* parent, int res, pointer curTime) : parent(parent), res(res), curTime(curTime){}

    friend bool operator==(const Iterator& l, const Iterator& r){return l.curTime >= r.curTime;}
    friend bool operator!=(const Iterator& l, const Iterator& r){return l.curTime != r.curTime;}
  private:
    FileSource* parent;
    pointer curTime;
    int res;
  };

  Iterator begin(){return Iterator(this, sampleResolution, 0); }
  Iterator end(){int endTime = (data.end()-1)->time - (data.begin())->time; return Iterator(this, sampleResolution, endTime); }
  float getLoad(int timePoint);
  bool canCharge(int timePoint);
  int getResolution(){return sampleResolution;}
  FileSource(std::string fname, geoFence gf);
  FileSource() = default;
  void setResolution(int res){sampleResolution = res;}
};

#endif
