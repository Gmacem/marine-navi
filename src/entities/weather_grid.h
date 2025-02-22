#pragma once
/*
#include "common.h"
#include "Ship.h"

#include <wx/curl/dialog.h>

#include <ctime>
#include <string>


namespace marine_navi::entities {


struct WeatherForecastData {
    geom::Point Point;

    double WaveHeight;

};

class IWeatherForecastProvider {
    using Point = geom::Point;
public:
    virtual WeatherForecastData GetForecast(time_t date, const Point& p) = 0;
    virtual ~IWeatherForecastProvider();
};

class EsimoProvider : public IWeatherForecastProvider {
    using Point = geom::Point;
public:
    EsimoProvider(std::string resourceId) {
        CURL *curl;
        CURLcode res;
    }

    WeatherForecastData GetForecast(time_t date, const Point& p) override;

    ~EsimoProvider() override = default;

private:
    const char* HOST = "http://esimo.ru/dataview/getresourceexport";

};

class WeatherGrid {
    using Point = geom::Point;
public:
    bool IsAvailablePoint(const Ship& ship, const Point& p) = 0;
};


} // namespace marine_navi::entities
*/