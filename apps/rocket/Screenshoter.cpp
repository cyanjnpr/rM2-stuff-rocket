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
    system(buildListCommand().c_str());
    auto list = unistdpp::readFile(default_list_path);
    if (list.has_value()) {
        std::istringstream stream(list.value());
        std::string line;
        while (std::getline(stream, line)) {
            if (line.find("'") == std::string::npos) notebooks_list.emplace_back(line);
        }
    } else {
        notebooks_list.emplace_back("");
    }
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
ScreenshoterState::buildCopyCommand() const {
    std::ostringstream builder;
    builder << "karmtka -g " 
        << getWidget().screenshot_edit_path 
        << " -m 0 -q " << quality 
        << " -x -i " << mode 
        << " -n '" << notebook
        << "' --overwrite";
    return builder.str();
}

std::string
ScreenshoterState::buildSimulateCommand() const {
    std::ostringstream builder;
    builder << "karmtka" 
        << " -x -i " << mode 
        << " -n '" << notebook
        << "' --overwrite --dry"
        << " > " << default_info_path;
    return builder.str();
}

std::string
ScreenshoterState::buildListCommand() const {
    std::ostringstream builder;
    builder << "karmtka" 
        << " -x -l"
        << " > " << default_list_path;
    return builder.str();
}
