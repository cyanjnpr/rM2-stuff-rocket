#include "Screenshoter.h"

#include <unistdpp/file.h>

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