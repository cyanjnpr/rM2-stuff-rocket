#pragma once

#include <UI.h>
#include <algorithm>
#include <sstream>

class ScreenshoterState;

class ScreenshoterWidget : public rmlib::StatefulWidget<ScreenshoterWidget> {
    public:
        ScreenshoterWidget(std::string screenshot_path, rmlib::Callback onClose)
            : screenshot_path(screenshot_path), screenshot_edit_path(screenshot_path + ".edit"), onClose(onClose) {
            screenshotImage = rmlib::ImageCanvas::load(screenshot_path.c_str());
        }

        static ScreenshoterState createState();

        rmlib::Callback onClose;
        std::string screenshot_path;
        std::string screenshot_edit_path;
        std::optional<rmlib::ImageCanvas> screenshotImage;
};

class ScreenshoterState : public rmlib::StateBase<ScreenshoterWidget> {

    public:
        void init(rmlib::AppContext& context, const rmlib::BuildContext&/*unused*/);

        auto screenshoter(rmlib::AppContext& context) const {
            using namespace rmlib;

            std::vector<DynamicWidget> widgets;

            auto screenshot = GestureDetector(Center(Image(getWidget().screenshotImage->canvas)),
                Gestures{}
                    .onTouchDown([this](Point pos) {
                        if (pos.y <= 100) return;
                        setState([pos](auto& self) {
                            self.touchDownPoint = pos;
                        });
                    })
                    .onTouchMove([this](Point pos) {
                        if (pos.y <= 100) return;
                        setState([pos](auto& self) {
                            self.selectionWidth = abs(self.touchDownPoint.x - pos.x);
                            self.selectionHeight = abs(self.touchDownPoint.y - pos.y);
                            self.selectionStart = { std::min(self.touchDownPoint.x, pos.x),
                                std::min(self.touchDownPoint.y, pos.y) };
                        });
                    })
                );
            auto copyButton = Button("Copy to Xochitl", [this, &context] {
                writeCroppedImage(getWidget().screenshot_edit_path.c_str(), 
                    selectionStart.x, selectionStart.y, selectionWidth, selectionHeight, getWidget().screenshotImage->canvas);
                std::ostringstream builder;
                builder << "karmtka -g " << getWidget().screenshot_edit_path << " -m 0 -x -i current --overwrite";
                system(builder.str().c_str());
                setState([] (auto& self) { self.resetSelection(); });
            }, default_text_size, 5, isValidSelection());
            auto closeButton = RoundButton("X", [this, &context] {
                getWidget().onClose(); 
            }, default_text_size, 10);
            auto controls = Corner(Border(Padding(Row(
                copyButton, closeButton
            ), Insets::sides(15, 5, 15, 15)), Insets::only_bottom(4)), 1);

            widgets.emplace_back(std::move(screenshot));
            if (isValidSelection()) {
                auto selection = Positioned(Border(Sized(Text(""), selectionWidth, selectionHeight),
                    Insets::all(2)), selectionStart);
                widgets.emplace_back(selection);
            }
            widgets.emplace_back(std::move(controls));

            return Stack(std::move(widgets), false, true);
        }

        auto build(rmlib::AppContext& context,
             const rmlib::BuildContext& /*unused*/) const {
            using namespace rmlib;

            auto ui = [&]() -> DynamicWidget {
                if (!getWidget().screenshotImage.has_value()) {
                    getWidget().onClose();
                    return Center(Text("Failed to capture screenshot."));
                }
                return screenshoter(context);
            }();

            return ui;
        }

    private:
        bool isValidSelection() const;
        void resetSelection();

        rmlib::Point touchDownPoint = {0, 0};
        rmlib::Point selectionStart = {0, 0};
        int selectionWidth = 0;
        int selectionHeight = 0;
};