#pragma once
#include <string>
#include "../enums/DayTime.hpp"


#include <functional>

class GameEngine;

class TimeCalendarManagers {
    DayTime dayTime = MORNING;

    int month = 6;
    int day = 1;
    GameEngine* engine = nullptr;

  public:
    void set_engine(GameEngine* eng) { engine = eng; }
    std::function<void(std::string)> on_popup;
    std::function<void(int)> on_day_advanced;

    DayTime getDayTime();

    void setDayTime(DayTime dayTime);

    std::string getTimeString();

    void incrementDay();

    void incrementMonth();

    int getDay();
    void setDay(int d) { day = d; }

    int getMonth();

    void advanceTime(bool is_double);
    void advanceDate();
};