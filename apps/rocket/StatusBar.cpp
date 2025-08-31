#include "StatusBar.h"

#include <unistdpp/file.h>
#include <sstream>
#include <fstream>

using namespace rmlib;

StatusBarState
StatusBarWidget::createState() {
    return StatusBarState{};
}

void
StatusBarState::init(rmlib::AppContext& context, 
        const rmlib::BuildContext& /*unused*/) {
    auto device_type_result = unistdpp::readFile(device_type_path);
    if (device_type_result.has_value()) {
        device_type = device_type_result.value();
        device_type.pop_back(); // nl
        if (device_type == "reMarkable 2.0") {
            charge_now_path = std::string(rm2_battery_path) + "charge_now";
            charge_full_path = std::string(rm2_battery_path) + "charge_full";
        } else if (device_type == "reMarkable Ferrari") {
            charge_now_path = std::string(rmpp_battery_path) + "charge_now";
            charge_full_path = std::string(rmpp_battery_path) + "charge_full";
        } else {
            charge_now_path = std::string(rm1_battery_path) + "charge_now";
            charge_full_path = std::string(rm1_battery_path) + "charge_full";
        }

        init_successful = readStatus();
    } else {
        init_successful = false;
    }
}

bool
StatusBarState::readStatus() {
    inactive_min = getWidget().inactive_min;
    return readUptime() && readBattery() && readTime();
}

bool
StatusBarState::readUptime() {
    std::ifstream uptime_file(uptime_path);
    std::string uptime;
    std::getline(uptime_file, uptime);
    auto uptime_result = unistdpp::readFile(uptime_path);
    if (!uptime.empty()) {
        uptime = uptime.substr(0, uptime.find("."));
        try {
            uptime_sec = std::stoi(uptime);
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }
    return false;
}

bool
StatusBarState::readBattery() {
    auto charge_full_result = unistdpp::readFile(charge_full_path);
    auto charge_now_result = unistdpp::readFile(charge_now_path);
    if (charge_full_result.has_value() && charge_now_result.has_value()) {
        std::string charge_full = charge_full_result.value();
        charge_full.pop_back(); // nl
        std::string charge_now = charge_now_result.value();
        charge_now.pop_back(); // nl
        try {
            charge_percent = 100 * std::stoi(charge_now) / std::stoi(charge_full);
            return true;
        } catch (const std::exception& e) {
            charge_percent = 0;
            return false;
        }
    }
    return false;
}

bool
StatusBarState::readTime() {
    auto t = std::time(nullptr);
    auto now = std::localtime(&t);
    char buf[6];
    std::strftime(buf, sizeof(buf), "%H:%M", now);
    current_time = std::string(buf);
    return true;
}

std::string
StatusBarState::formatStatus() const {
    int uptime_m = uptime_sec / 60 % 60;
    int uptime_h = uptime_sec / 60 / 60 % 24;
    int uptime_d = uptime_sec / 60 / 60 / 24;
    std::ostringstream builder;
    builder << "Battery "
        << charge_percent << "% | "
        << "Up ";
    if (uptime_d > 0) builder << uptime_d << "d";
    else if (uptime_h > 0) builder << uptime_h << "h";
    else builder << uptime_m << "m";
    builder << " | Inactive " << inactive_min << "m"
        << " | Time " << current_time << " UTC";

    for (int i = 0; i < 64; i++) {
        // fill so the widget takes full width of the screen
        // then border will be displayed nicely as a separator
        builder << " ";
    }
    return builder.str();
}
