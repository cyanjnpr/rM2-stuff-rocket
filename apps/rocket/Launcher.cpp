#include "Launcher.h"

#include <unistdpp/file.h>

using namespace rmlib;

namespace {

constexpr std::array static_app_paths = { "/opt/etc/draft", "/etc/draft" };

#ifndef KEY_POWER
#define KEY_POWER 116
#endif

} // namespace

LauncherState
LauncherWidget::createState() {
  return LauncherState{};
}

void
LauncherState::init(rmlib::AppContext& context,
                    const rmlib::BuildContext& /*unused*/) {
  suspendedImage = rmlib::ImageCanvas::load(default_suspended_path);

  if (auto* key = context.getInputManager().getBaseDevices().key;
      key != nullptr) {
    key->grab();
  }

  fbCanvas = &context.getFbCanvas();
  touchDevice = context.getInputManager().getBaseDevices().touch;

  readApps();

  context.listenFd(AppManager::getInstance().getWaitFD().fd, [this] {
    setState([](auto& self) { self.updateStoppedApps(); });
  });

  inactivityTimer = context.addTimer(
    std::chrono::minutes(1),
    [this, &context] {
      inactivityCountdown -= 1;
      if (inactivityCountdown == 0) {
        resetInactivity();
        setState([&context](auto& self) {
          self.startTimer(context);
          self.show();
        });
      }
    },
    std::chrono::minutes(1));
}

bool
LauncherState::sleep() {
  lock();
  system("/sbin/rmmod brcmfmac");
  int res = system("echo \"mem\" > /sys/power/state");
  system("/sbin/modprobe brcmfmac");

  if (res == 0) {
    // Get the reason
    auto irq = unistdpp::readFile("/sys/power/pm_wakeup_irq");
    if (!irq.has_value()) {
      std::cout << "Error getting reason: " << unistdpp::to_string(irq.error())
                << std::endl;

      // If there is no irq it must be the user which pressed the button:
      return true;
    }
    std::cout << "Reason for wake irq: " << *irq << std::endl;
    return false;
  }

  return false;
}

void
LauncherState::stopTimer() {
  sleepTimer.disable();
  sleepCountdown = -1;
}

void
LauncherState::startTimer(rmlib::AppContext& context, int time) {
  sleepCountdown = time;
  sleepTimer = context.addTimer(
    std::chrono::seconds(time == 0 ? 0 : 1),
    [this] { tick(); },
    std::chrono::seconds(1));
}

void
LauncherState::tick() const {
  setState([](auto& self) {
    self.sleepCountdown -= 1;

    if (self.sleepCountdown == -1) {
      if (self.sleep()) {
        // If the user pressed the button, stop the timer and return to the
        // current app.
        self.resetInactivity();
        self.sleepTimer.disable();
        self.show();
        // self.hide(nullptr);
      } else {
        // Retry sleeping if failed or something else woke us.
        self.sleepCountdown = retry_sleep_timeout;
      }
    }
  });
}

void LauncherState::long_press(rmlib::AppContext& context) {
  if (!isUnlocked) return;

  if (visible) {
    stopTimer();
    std::string highlightedAppPath = previousAppPath.size() > 0 ? previousAppPath : currentAppPath;
    int firstActiveIndex = -1;
    int currentHighlightIndex = -1;
    
    for (size_t i = 0; i < apps.size(); i++) {
      if (firstActiveIndex == -1 && apps[i].isRunning()) {
        previousAppPath = apps[i].description().path;
        firstActiveIndex = i;
      }
      if (currentHighlightIndex == -1 && apps[i].description().path == highlightedAppPath) {
        currentHighlightIndex = i;
      } else if (currentHighlightIndex > -1 && apps[i].isRunning()) {
        previousAppPath = apps[i].description().path;
        break;
      }
    }
  } else if (!isViewingScreenshot) {
    if (writeImage(default_screenshot_path, *fbCanvas)) {
      std::cout << "Screenshot saved at " <<  default_screenshot_path << std::endl;
      if (!visible) show();
      showScreenshot();
    }
  }
}

void LauncherState::short_press(rmlib::AppContext& context) {
  if (!isUnlocked) return;

  if (isViewingScreenshot) hideScreenshot();
  else toggle(context);
}

void LauncherState::lock() {
  isManagingPower = false;
  isUnlocked = false;
}

void LauncherState::unlock() {
  isUnlocked = true;
  resetInactivity();
}

void LauncherState::showScreenshot() {
  isViewingScreenshot = true;
  stopTimer();
}

void LauncherState::hideScreenshot() {
  isViewingScreenshot = false;
}

void
LauncherState::toggle(rmlib::AppContext& context) {
  isManagingPower = false;
  if (visible) {
    if (!isUnlocked) {
      // something breaks if button is clicked a few times with timer == 0
      // it's not driver unloading/loading, placing mutex on sleep() call doesn't help
      // startTimer(context, 0);
      return;
    }
    bool shouldStartTimer = sleepCountdown <= 0;
    stopTimer();
    hide(shouldStartTimer ? &context : nullptr);
  } else {
    startTimer(context);
    show();
  }
}

void
LauncherState::show() {
  if (visible) {
    return;
  }

  if (auto* current = getCurrentApp(); current != nullptr) {
    current->pause(MemoryCanvas(*fbCanvas));
  }

  readApps();
  visible = true;
}

void
LauncherState::hide(rmlib::AppContext* context) {
  if (!visible) {
    return;
  }
  if (auto* current = getPreviousApp(); current != nullptr) {
    switchApp(*current);
  } else if (auto* current = getCurrentApp(); current != nullptr) {
    switchApp(*current);
  } else if (context != nullptr) {
    startTimer(*context, 0);
  }
}

void
LauncherState::deviceStatus() const {
  
}

App*
LauncherState::getPreviousApp() {
  auto app = std::find_if(apps.begin(), apps.end(), [this](auto& app) {
    if (previousAppPath.size() == 0) {
      return false;
    }
    return app.description().path == previousAppPath;
  });

  if (app == apps.end()) {
    return nullptr;
  }

  return &*app;
}

const App*
LauncherState::getPreviousApp() const {
  // NOLINTNEXTLINE
  return const_cast<LauncherState*>(this)->getPreviousApp();
}

App*
LauncherState::getCurrentApp() {
  auto app = std::find_if(apps.begin(), apps.end(), [this](auto& app) {
    return app.description().path == currentAppPath;
  });

  if (app == apps.end()) {
    return nullptr;
  }

  return &*app;
}

const App*
LauncherState::getCurrentApp() const {
  // NOLINTNEXTLINE
  return const_cast<LauncherState*>(this)->getCurrentApp();
}

void
LauncherState::switchApp(App& app) {
  visible = false;
  stopTimer();

  // Pause the current app.
  if (auto* currentApp = getCurrentApp(); currentApp != nullptr &&
                                          currentApp->isRunning() &&
                                          !currentApp->isPaused()) {
    currentApp->pause();
  }

  // resume or launch app
  if (app.isPaused()) {
    if (touchDevice != nullptr) {
      touchDevice->flood();
    }
    app.resume();
  } else if (!app.isRunning()) {
    app.resetSavedFB();

    if (!app.launch()) {
      std::cerr << "Error launching " << app.description().command << std::endl;
      return;
    }
  }

  previousAppPath = currentAppPath;
  currentAppPath = app.description().path;
}

void
LauncherState::updateStoppedApps() {
  AppManager::getInstance().update();

  bool shouldShow = false;
  if (const auto* current = getCurrentApp();
      current != nullptr && !current->isRunning()) {
    currentAppPath = "";
    shouldShow = true;
  }
  if (const auto* current = getPreviousApp();
      current != nullptr && !current->isRunning()) {
    previousAppPath = "";
  }

  apps.erase(std::remove_if(apps.begin(),
                            apps.end(),
                            [](const App& app) {
                              return app.shouldRemoveOnExit() &&
                                     !app.isRunning();
                            }),
             apps.end());

  if (shouldShow) {
    show();
  }
}

void
LauncherState::readApps() {
  const static auto app_paths = [] {
    std::vector<std::string> paths;
    std::transform(static_app_paths.begin(),
                   static_app_paths.end(),
                   std::back_inserter(paths),
                   [](const auto* str) { return std::string(str); });

    if (const auto* home = getenv("HOME"); home != nullptr) {
      paths.push_back(std::string(home) + "/.config/draft");
    }

    return paths;
  }();

  std::vector<AppDescription> appDescriptions;
  for (const auto& appsPath : app_paths) {
    auto decriptions = readAppFiles(appsPath);
    std::move(decriptions.begin(),
              decriptions.end(),
              std::back_inserter(appDescriptions));
  }

  // Update known apps.
  for (auto appIt = apps.begin(); appIt != apps.end();) {

    auto descIt = std::find_if(appDescriptions.begin(),
                               appDescriptions.end(),
                               [&app = *appIt](const auto& desc) {
                                 return desc.path == app.description().path;
                               });

    // Remove old apps.
    if (descIt == appDescriptions.end()) {
      if (!appIt->isRunning()) {
        appIt = apps.erase(appIt);
        continue;
      }

      // Defer removing until exit.
      appIt->setRemoveOnExit();

    } else {

      // Update existing apps.
      appIt->updateDescription(std::move(*descIt));
      appDescriptions.erase(descIt);
    }

    ++appIt;
  }

  // Any left over descriptions are new.
  for (auto& desc : appDescriptions) {
    apps.emplace_back(std::move(desc));
  }

  std::sort(apps.begin(), apps.end(), [](const auto& app1, const auto& app2) {
    return app1.description().path < app2.description().path;
  });
}

void
LauncherState::resetInactivity() const {
  // sleep sooner if locked, at countdown == 1 it doesn't work as it should
  inactivityCountdown = isUnlocked ? default_inactivity_timeout : 2;
}
