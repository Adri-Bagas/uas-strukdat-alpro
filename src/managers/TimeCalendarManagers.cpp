#include "TimeCalendarManagers.hpp"
#include "../utils/Logger.hpp"
#include "../utils/components/Popup.hpp"

DayTime TimeCalendarManagers::getDayTime() {
    return dayTime;
}
void TimeCalendarManagers::setDayTime(DayTime dayTime) {
    this->dayTime = dayTime;
    
    std::string phase = getTimeString();
    Popup p {"Phase changed to: " + phase};
    p.animate();
    p.type_text();
}

void TimeCalendarManagers::incrementDay() {
    day++;
    Popup p {"A new day has dawned: Day " + std::to_string(day)};
    p.animate();
    p.type_text();
}

void TimeCalendarManagers::incrementMonth() {
    month++;
    Popup p {"A new month has begun: Month " + std::to_string(month)};
    p.animate();
    p.type_text();
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
        return "Pagi";
    case AFTERNOON:
        return "Siang";
    case EVENING:
        return "Malam";
    default:
        return "Tidak Diketahui";
    }
}

void TimeCalendarManagers::advanceTime(bool is_double) {
    DayTime oldTime = this->dayTime;
    switch (this->dayTime) {
    case MORNING:
        if (is_double) {
            this->dayTime = EVENING;
        } else {
            this->dayTime = AFTERNOON;
        }
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
        return;
    default:
        Logger::log("Error: Kesalahan saat memajukan waktu!");
        return;
    }

    if (oldTime != this->dayTime) {
        std::string phase = getTimeString();
        Popup p {"Waktu telah berlalu. Sekarang waktu " + phase};
        p.animate();
        p.type_text();
    }
}

void TimeCalendarManagers::advanceDate() {
    this->day += 1;
    this->dayTime = MORNING;
    Popup p {"Hari telah berakhir. Kamu terbangun pada Hari ke-" + std::to_string(this->day)};
    p.animate();
    p.type_text();
}