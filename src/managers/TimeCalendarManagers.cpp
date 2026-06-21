#include "TimeCalendarManagers.hpp"
#include "../utils/Logger.hpp"
#include "../utils/components/Popup.hpp"

DayTime TimeCalendarManagers::getDayTime() {
    return dayTime;
}
void TimeCalendarManagers::setDayTime(DayTime dayTime) {
    this->dayTime = dayTime;
    
    std::string phase = getTimeString();
    if (on_popup) on_popup("Phase changed to: " + phase);
}

void TimeCalendarManagers::incrementDay() {
    day++;
    if (on_popup) on_popup("A new day has dawned: Day " + std::to_string(day));
}

void TimeCalendarManagers::incrementMonth() {
    month++;
    if (on_popup) on_popup("A new month has begun: Month " + std::to_string(month));
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
        if (on_popup) on_popup("Waktu telah berlalu. Sekarang waktu " + phase);
    }
}

void TimeCalendarManagers::advanceDate() {
    this->day += 1;
    this->dayTime = MORNING;
    if (on_popup) on_popup("Hari telah berakhir. Kamu terbangun pada Hari ke-" + std::to_string(this->day));
    
    if (on_day_advanced) {
        // Simple day of week calculation (1 to 7)
        int day_of_week = ((this->day - 1) % 7) + 1;
        on_day_advanced(day_of_week);
    }
}