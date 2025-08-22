#include "Screenshoter.h"

#include <unistdpp/file.h>
#include <sstream>

using namespace rmlib;

ScreenshoterState
ScreenshoterWidget::createState() {
    return ScreenshoterState{};
}

void
ScreenshoterState::init(rmlib::AppContext& context, 
        const rmlib::BuildContext& /*unused*/) {

}

bool
ScreenshoterState::isValidSelection() const {
    return selectionHeight > 0 && selectionWidth > 0;
}

void
ScreenshoterState::resetSelection() {
    selectionHeight = 0;
    selectionWidth = 0;
    selectionStart = {0, 0};
}

// hardcoded for now, it's a static widget anyway
bool 
ScreenshoterState::isInControls(rmlib::Point pos) const {
    return pos.y <= 100 && pos.x >= 650;
}

std::string 
ScreenshoterState::buildCopyCommand(std::string quality, std::string mode) const {
    std::ostringstream builder;
    builder << "karmtka -g " 
        << getWidget().screenshot_edit_path 
        << " -m 0 -q " << quality 
        << " -x -i " << mode 
        << " --overwrite";
    return builder.str();
}
