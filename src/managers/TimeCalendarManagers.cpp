#include "TimeCalendarManagers.hpp"
#include "../utils/Logger.hpp"

DayTime TimeCalendarManagers::getDayTime() {
    return dayTime;
}
void TimeCalendarManagers::setDayTime(DayTime dayTime) {
    this->dayTime = dayTime;
}

void TimeCalendarManagers::incrementDay() {
    day++;
}

void TimeCalendarManagers::incrementMonth() {
    month++;
}

int TimeCalendarManagers::getDay() {
    return day;
}

int TimeCalendarManagers::getMonth() {
    return month;
}

std::string TimeCalendarManagers::getTimeString() {
    switch (this->dayTime) {
    case MORNING:
        return "Morning";
    case AFTERNOON:
        return "Afternoon";
    case EVENING:
        return "Evening";
    default:
        return "Unknown";
    }
}

void TimeCalendarManagers::advanceTime(bool is_double) {
    switch (this->dayTime) {
    case MORNING:
        if (is_double) {
            this->dayTime = EVENING;
            return;
        }
        this->dayTime = AFTERNOON;
        break;
    case AFTERNOON:
        if (is_double) {
            advanceDate();
            return;
        }
        this->dayTime = EVENING;
        break;
    case EVENING:
        advanceDate();
        break;
    default:
        Logger::log("Error: Error Advencing time outside of the boundaries!");
    }
}

void TimeCalendarManagers::advanceDate() {
    this->day += 1;
}