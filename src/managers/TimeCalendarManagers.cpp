#include "TimeCalendarManagers.hpp"

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