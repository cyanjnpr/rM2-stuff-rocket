#pragma once

#include "App.h"
#include "AppWidgets.h"

#include <UI.h>

class LauncherState;

class LauncherWidget : public rmlib::StatefulWidget<LauncherWidget> {
public:
  static LauncherState createState();
};

class LauncherState : public rmlib::StateBase<LauncherWidget> {
  constexpr static auto default_sleep_timeout = 10;
  constexpr static auto retry_sleep_timeout = 8;
  constexpr static auto default_inactivity_timeout = 20;

  constexpr static rmlib::Size splash_size = { 512, 512 };

public:
  void init(rmlib::AppContext& context, const rmlib::BuildContext& /*unused*/);

  auto header(rmlib::AppContext& context) const {
    using namespace rmlib;

    const auto text = [this]() -> std::string {
      switch (sleepCountdown) {
        case -1:
          return "Welcome";
        case 0:
          return "Sleeping";
        default:
          return "Sleeping in : " + std::to_string(sleepCountdown);
      }
    }();

    auto button = [this, &context] {
      if (sleepCountdown > 0) {
        return Button(
          "Stop", [this] { setState([](auto& self) { self.stopTimer(); }); });
      }
      if (sleepCountdown == 0) {
        // TODO: make hideable?
        return Button("...", [] {});
      }
      return Button("Sleep", [this, &context] {
        setState([&context](auto& self) { self.startTimer(context, 0); });
      });
    }();

    return Center(Padding(
      Column(Padding(Text(text, 2 * default_text_size), Insets::all(10))),
           // button),
      Insets::all(50)));
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

  auto launcher(rmlib::AppContext& context) const {
    using namespace rmlib;

    return Cleared(Column(header(context), runningApps(), appList()));
  }

  auto lockscreen(rmlib::AppContext& context) const {
    using namespace rmlib;

    std::string passcodeCopy = passcode;
    std::fill(passcodeCopy.begin(), passcodeCopy.end(), '*');

    auto unlockText = Text("Enter passcode:", 2 * default_text_size);
    auto passcodeText = Text(passcodeCopy, (int)(2 * default_text_size));

    auto b1 = Button(
      "1", [this] { setState([](auto& self) { self.typePasscode('1'); }); }, 
      default_text_size * 1.5, 15);
    auto b2 = Button(
      "2", [this] { setState([](auto& self) { self.typePasscode('2'); }); }, 
      default_text_size * 1.5, 15);
    auto b3 = Button(
      "3", [this] { setState([](auto& self) { self.typePasscode('3');}); }, 
      default_text_size * 1.5, 15);
    auto b4 = Button(
      "4", [this] { setState([](auto& self) { self.typePasscode('4'); }); }, 
      default_text_size * 1.5, 15);
    auto b5 = Button(
      "5", [this] { setState([](auto& self) { self.typePasscode('5'); }); }, 
      default_text_size * 1.5, 15);
    auto b6 = Button(
      "6", [this] { setState([](auto& self) { self.typePasscode('6'); }); }, 
      default_text_size * 1.5, 15);
    auto b7 = Button(
      "7", [this] { setState([](auto& self) { self.typePasscode('7'); }); }, 
      default_text_size * 1.5, 15);
    auto b8 = Button(
      "8", [this] { setState([](auto& self) { self.typePasscode('8'); }); }, 
      default_text_size * 1.5, 15);
    auto b9 = Button(
      "9", [this] { setState([](auto& self) { self.typePasscode('9'); }); }, 
      default_text_size * 1.5, 15);
    auto b0 = Button(
      "0", [this] { setState([](auto& self) { self.typePasscode('0'); }); }, 
      default_text_size * 1.5, 15);
    auto bClear = Button(
      "<=", [this] { setState([](auto& self) { if (!self.passcode.empty()) {
        self.passcode.pop_back();
      } }); }, 
      default_text_size * 1.5, 15);

    return Cleared(Column(
      Padding(unlockText, Insets::all(50)),
      Padding(passcodeText, Insets::all(100)), 
      Row(Padding(b1, Insets::all(30)), Padding(b2, Insets::all(30)), Padding(b3, Insets::all(30))),
      Row(Padding(b4, Insets::all(30)), Padding(b5, Insets::all(30)), Padding(b6, Insets::all(30))),
      Row(Padding(b7, Insets::all(30)), Padding(b8, Insets::all(30)), Padding(b9, Insets::all(30))),
      Row(Padding(b0, Insets::all(30)), Padding(bClear, Insets::all(30)))
    ));
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
      if (visible) {
        if (sleepCountdown == 0) {
          // SEGFAULTS based on image parameters unknown to me
          // possibly color profile
          // xkcd comics work fine so do draft icons
          // auto img = ImageCanvas::load("/home/root/wallpapers/suspended.png");
          // if (img.has_value()) {
          //   const Canvas& canvas = img->canvas;
          //   return Center(Sized(Image(canvas), 1404, 1872));
          // }
          return Center(Text("Zzz...", 2 * default_text_size));
        }
        if (isPasscodeGood || xochitlPasscode.size() == 0) {
          return launcher(context);
        }
        return lockscreen(context);
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
            setState([&context](auto& self) { self.toggle(context); });
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

  void typePasscode(char rune);

  std::vector<App> apps;
  std::string currentAppPath;
  std::string previousAppPath;

  std::string passcode;
  std::string xochitlPasscode;
  bool isPasscodeGood;
  int attempts;

  std::optional<rmlib::MemoryCanvas> backupBuffer;

  rmlib::TimerHandle sleepTimer;
  rmlib::TimerHandle inactivityTimer;

  const rmlib::Canvas* fbCanvas = nullptr;
  rmlib::input::InputDeviceBase* touchDevice = nullptr;

  int sleepCountdown = -1;
  mutable int inactivityCountdown = default_inactivity_timeout;
  bool visible = true;
};
