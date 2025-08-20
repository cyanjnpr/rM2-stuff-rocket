#include "Lockscreen.h"

#include <unistdpp/file.h>

using namespace rmlib;

namespace {
    const std::string xochitl_conf_path = "/home/root/.config/remarkable/xochitl.conf";
}

LockscreenState
LockscreenWidget::createState() {
    return LockscreenState{};
}

void
LockscreenState::init(rmlib::AppContext& context, 
        const rmlib::BuildContext& /*unused*/) {
  attempts = 0;
  xochitlPasscode = "";

  unistdpp::Result<std::string> result = unistdpp::readFile(std::filesystem::path(xochitl_conf_path));
  if (result.has_value()) {
    std::istringstream stream(result.value());
    std::string line;
    std::string prefix = "Passcode";

    while (std::getline(stream, line)) {
        if (line.compare(0, prefix.size(), prefix) == 0) {
          size_t pos = line.find('=');
          if (pos != std::string::npos && pos + 1 < line.size()) {
            xochitlPasscode = line.substr(pos + 1); 
            break;
          }
        }
    }
  }
}

void LockscreenState::typePasscode(char rune) {
  passcode += rune;
  if (passcode.size() >= xochitlPasscode.size()) {
    isPasscodeGood = passcode == xochitlPasscode;
    attempts++;
    passcode = "";
    if (isPasscodeGood) {
      getWidget().onUnlock();
      attempts = 0;
    } else if (attempts >= 5) {
      system("/sbin/poweroff");
    }
  }
}
