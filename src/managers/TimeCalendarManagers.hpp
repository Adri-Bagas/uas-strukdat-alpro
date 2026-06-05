#pragma once
#include <string>
#include "../enums/DayTime.hpp"


class TimeCalendarManagers {
    DayTime dayTime = MORNING;

    int month = 6;
    int day = 1;

  public:
    DayTime getDayTime();

    void setDayTime(DayTime dayTime);

    std::string getTimeString();

    void incrementDay();

    void incrementMonth();

    int getDay();

    int getMonth();

    void advanceTime(bool is_double);
    void advanceDate();
};