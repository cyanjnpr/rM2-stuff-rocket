#include "Lockscreen.h"

#include <unistdpp/file.h>

using namespace rmlib;

LockscreenState
LockscreenWidget::createState() {
    return LockscreenState{};
}

void
LockscreenState::init(rmlib::AppContext& context, 
        const rmlib::BuildContext& /*unused*/) {
  attempts = 0;
  passcode = "";
  isPasscodeGood = false;
}

void LockscreenState::typePasscode(char rune) {
  passcode += rune;
  if (passcode.size() >= getWidget().xochitlPasscode.size()) {
    isPasscodeGood = passcode == getWidget().xochitlPasscode;
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
