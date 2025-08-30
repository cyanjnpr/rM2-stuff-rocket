#pragma once

#include <UI.h>
#include <chrono>

#include "App.h"
#include "AppWidgets.h"

#include "Lockscreen.h"
#include "Screenshoter.h"

class LauncherState;

class LauncherWidget : public rmlib::StatefulWidget<LauncherWidget> {
public:
  static LauncherState createState();
};

class LauncherState : public rmlib::StateBase<LauncherWidget> {
  constexpr static auto default_sleep_timeout = 10;
  constexpr static auto retry_sleep_timeout = 8;
  constexpr static auto default_inactivity_timeout = 20;
  const char* default_screenshot_path = "/tmp/rocket_screenshot.png";
  const char* default_suspended_path = "/usr/share/remarkable/suspended.png";

  constexpr static rmlib::Size splash_size = { 512, 512 };

public:
  void init(rmlib::AppContext& context, const rmlib::BuildContext& /*unused*/);

  auto header(/*rmlib::AppContext& context*/) const {
    using namespace rmlib;

    const auto text = [this]() -> std::string {
      switch (sleepCountdown) {
        case -1:
          return "";
        default:
          return "Sleeping in: " + std::to_string(sleepCountdown);
      }
    }();

    return Padding(Text(text), Insets::all(10));
  }

  auto runningApps() const {
    using namespace rmlib;

    std::vector<RunningAppWidget> widgets;
    for (const auto& app : apps) {
      if (app.isRunning()) {
        widgets.emplace_back(
          app,
          [this, &app] {
            setState(
              [&app](auto& self) { self.switchApp(*const_cast<App*>(&app)); });
          },
          [this, &app] {
            setState([&app](auto& self) {
              std::cout << "stopping " << app.description().name << std::endl;
              const_cast<App*>(&app)->stop();
              self.stopTimer();
            });
          },
          previousAppPath.size() > 0 ? 
            app.description().path == previousAppPath : 
            app.description().path == currentAppPath);
      }
    }
    return Wrap(widgets);
  }

  auto appList() const {
    using namespace rmlib;

    std::vector<AppWidget> widgets;
    for (const auto& app : apps) {
      if (!app.isRunning()) {
        widgets.emplace_back(app, [this, &app] {
          setState(
            [&app](auto& self) { self.switchApp(*const_cast<App*>(&app)); });
        });
      }
    }
    return Wrap(widgets);
  }

  auto powerManagementDialog() const {
    using namespace rmlib;
    int spacing = 30;

    return Center(Cleared(Border(Column(
        Padding(Row(Text(managementOperation + " the device?")), Insets::all(spacing)),
        Padding(Row(
            Padding(Button("Cancel", [this]{
                setState([](auto& self) { 
                    self.isManagingPower = false; });
        }), Insets::all(spacing)), 
            Padding(Button("Confirm", [this]{
                std::string command = "/sbin/" + managementOperation;
                system(command.c_str());
            }), Insets::all(spacing))
        ), Insets::all(spacing))
    ), Insets::all(4))));
  }

  auto launcher(rmlib::AppContext& context) const {
    using namespace rmlib;

    std::vector<DynamicWidget> widgets;
    auto controls = Corner(Column(
        Button("Shutdown", [this] { 
          setState([](auto& self) {
            self.managementOperation = "poweroff"; self.isManagingPower = true; 
          }); }),
        Button("Reboot  ", [this] { 
          setState([](auto& self) {
            self.managementOperation = "reboot"; self.isManagingPower = true; 
          }); })
      ), 0);
    auto head = Corner(header(), 1);
    auto menu = Center(Column(runningApps(), appList()));
    widgets.emplace_back(std::move(controls));
    widgets.emplace_back(std::move(head));
    widgets.emplace_back(std::move(menu));
    if (isManagingPower) {
      widgets.emplace_back(std::move(powerManagementDialog()));
    }

    return Stack(std::move(widgets), isManagingPower, true);
  }

  auto build(rmlib::AppContext& context,
             const rmlib::BuildContext& /*unused*/) const {
    using namespace rmlib;
    
    const Canvas* background = nullptr;
    std::optional<Size> backgroundSize = {};
    if (const auto* currentApp = getCurrentApp(); currentApp != nullptr) {
      if (const auto& savedFb = currentApp->savedFB(); savedFb.has_value()) {
        background = &savedFb->canvas;
      } else if (const auto& icon = currentApp->icon(); icon.has_value()) {
        background = &icon->canvas;
        backgroundSize = splash_size;
      }
    }

    auto ui = [&]() -> DynamicWidget {
      if (isViewingScreenshot) {
        return ScreenshoterWidget(default_screenshot_path, [this] {
          setState([](auto& self) { self.isViewingScreenshot = false; });
        });
      }
      if (visible) {
        if (sleepCountdown == 0) {
          if (suspendedImage.has_value()) {
            return Cleared(Center(Image(suspendedImage->canvas)));
          }

          return Cleared(Center(Text("Zzz...", 2 * default_text_size)));
        }

        LockscreenWidget lockscreen = LockscreenWidget([this] {
          setState([](auto& self) { self.unlock(); });
        });
        if (isUnlocked) { return launcher(context); }
        return lockscreen;
      }

      if (background == nullptr) {
        return Colored(white);
      }

      if (backgroundSize.has_value()) {
        return Center(Sized(
          Image(*background), backgroundSize->width, backgroundSize->height));
      }
      return Image(*background);
    }();

    return GestureDetector(
      std::move(ui),
      Gestures{}
        .onKeyDown([this, &context](auto keyCode) {
          if (keyCode == KEY_POWER) {
            setState([&context](auto& self) { 
              std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
              self.lastKeyPress = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
             });
          }
        })
        .onKeyUp([this, &context](auto keyCode) {
          if (keyCode == KEY_POWER) {
            setState([this, &context](auto& self) { 
              std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
              if (std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count() - self.lastKeyPress > long_press_ms) {
                self.long_press(context);
              } else {
                self.short_press(context);
              }
             });
          }
        })
        .onAny([this]() { resetInactivity(); }));
  }

private:
  bool sleep();

  void startTimer(rmlib::AppContext& context, int time = default_sleep_timeout);
  void stopTimer();

  void tick() const;

  void show();
  void hide(rmlib::AppContext* context);
  void toggle(rmlib::AppContext& context);

  App* getCurrentApp();
  const App* getCurrentApp() const;

  App* getPreviousApp();
  const App* getPreviousApp() const;

  void switchApp(App& app);

  void updateStoppedApps();

  void readApps();

  void resetInactivity() const;

  void deviceStatus() const;

  void lock();
  void unlock();
  void showScreenshot();
  void hideScreenshot();

  void short_press(rmlib::AppContext& context);
  void long_press(rmlib::AppContext& context);

  std::string managementOperation = "poweroff";
  bool isManagingPower = false;

  int64_t lastKeyPress = 0;
  const int long_press_ms = 400;

  bool isUnlocked = false;
  bool isViewingScreenshot = false;

  std::vector<App> apps;
  std::string currentAppPath;
  std::string previousAppPath;

  std::optional<rmlib::ImageCanvas> suspendedImage;
  std::optional<rmlib::MemoryCanvas> backupBuffer;

  rmlib::TimerHandle sleepTimer;
  rmlib::TimerHandle inactivityTimer;

  const rmlib::Canvas* fbCanvas = nullptr;
  rmlib::input::InputDeviceBase* touchDevice = nullptr;

  int sleepCountdown = -1;
  mutable int inactivityCountdown = default_inactivity_timeout;
  bool visible = true;
};
