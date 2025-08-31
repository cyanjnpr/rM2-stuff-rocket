#pragma once

#include <unistdpp/file.h>
#include <UI.h>

namespace {
    // https://github.com/koreader/koreader/blob/master/frontend/device/remarkable/device.lua
    const char* device_type_path = "/sys/devices/soc0/machine";
    const char* rm1_battery_path = "/sys/class/power_supply/bq27441-0/";
    const char* rm2_battery_path = "/sys/class/power_supply/max77818_battery/";
    const char* rmpp_battery_path = "/sys/class/power_supply/max1726x_battery/";

    const char* uptime_path = "/proc/uptime";
}

class StatusBarState;

class StatusBarWidget : public rmlib::StatefulWidget<StatusBarWidget> {
    public:
        StatusBarWidget(int inactive_min) : inactive_min(inactive_min)  {
        }

        static StatusBarState createState();

    int inactive_min = 0;
};

class StatusBarState : public rmlib::StateBase<StatusBarWidget> {

    public:
        void init(rmlib::AppContext& context, const rmlib::BuildContext&/*unused*/);


        auto build(rmlib::AppContext& context,
             const rmlib::BuildContext& /*unused*/) const {
            using namespace rmlib;

            if (init_successful && inactive_min != getWidget().inactive_min) {
                setState([this](auto& self){
                    self.readStatus();
                });
            }

            auto ui = [&]() -> DynamicWidget {
                if (init_successful) {
                    return Border(Padding(Text(formatStatus()), 
                        Insets::sides(0, 10, 10, 0)), Insets::only_top(2));
                }
                return Border(Padding(Text("failed to init the status bar"), 
                        Insets::sides(0, 10, 10, 0)), Insets::sides(2, 0, 0, 2));
            }();

            return ui;
        }

    private:
        bool readStatus();
        bool readUptime();
        bool readBattery();
        bool readTime();
        std::string formatStatus() const;

        std::string device_type;
        std::string charge_full_path;
        std::string charge_now_path;
        std::string current_time;

        int charge_percent = 0;
        int uptime_sec = 0;
        int inactive_min = 0;

        bool init_successful = false;
};
