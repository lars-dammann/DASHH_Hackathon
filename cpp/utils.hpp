#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <tuple>

constexpr float secsPerHour = 60 * 60;
constexpr float secsPerDay  = 60 * 60 * 24;
constexpr float secsPerYear = 60 * 60 * 24 * 365;

//#include "components.hpp"

using chargeTime = std::pair<int,int>;
struct geoFence{float minLat; float maxLat; float minLon; float maxLon;};

#endif
