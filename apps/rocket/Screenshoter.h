#pragma once

#include <UI.h>
#include <UI/Navigator.h>
#include <algorithm>
#include <sstream>

namespace {

constexpr std::array inject_modes = { "current", "append", "next", "last" };

}

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

        auto copy_dialog(rmlib::AppContext& context) const {
            using namespace rmlib;
            int spacing = 25;
            
            return Center(Cleared(Border(Column(
                Padding(Row(Text("A few last options")), Insets::all(spacing)),
                Padding(Row(
                    SpinWidget("Screenshot Quality", [this](std::string value) {
                        setState([value](auto& self) { self.quality = value; });
                    }, 2, 255, 1, 50
                )), Insets::all(spacing)),
                Padding(Row(
                    SpinWidget("Inject Mode", [this](std::string value) {
                        setState([value](auto& self) { self.mode = value; });
                    }, 0, 3, 1, 1, [](int val) {
                        return inject_modes[val];
                    }
                )), Insets::all(spacing)),
                Padding(Row(
                    Padding(Button("Cancel", [this] {
                        setState([](auto& self) { 
                            self.resetSelection(); 
                            self.isSelecting = true; });
                    }), Insets::all(spacing)),
                    Padding(Button("Copy", [this] {
                        std::string command = buildCopyCommand(quality, mode);
                        system(command.c_str());
                        getWidget().onClose();
                    }), Insets::all(spacing))
                ), Insets::all(spacing))
            ), Insets::all(4))));
        }

        auto screenshot_controls(rmlib::AppContext& context) const {
            using namespace rmlib;

            auto copyButton = Padding(Button("Copy to Xochitl", [this, &context] {
                writeCroppedImage(getWidget().screenshot_edit_path.c_str(), 
                    selectionStart.x, selectionStart.y, selectionWidth, selectionHeight, getWidget().screenshotImage->canvas);
                setState([] (auto& self) { 
                    self.isSelecting = false;
                    // match state with ui
                    self.quality = "2";
                    self.mode = inject_modes[0];
                });
            }, default_text_size, 5, isValidSelection()), Insets::only_left(10));
            auto closeButton = Padding(RoundButton("X", [this, &context] {
                getWidget().onClose(); 
            }, default_text_size, 10), Insets::only_left(10));
            auto controls = Corner(Cleared(Border(Padding(Row(
                Text("Screenshot"), copyButton, closeButton
            ), Insets::sides(15, 5, 15, 15)), Insets::sides(0, 4, 4, 0))), 1);

            return controls;
        }

        auto screenshoter(rmlib::AppContext& context) const {
            using namespace rmlib;

            std::vector<DynamicWidget> widgets;

            auto screenshot = GestureDetector(Center(Image(getWidget().screenshotImage->canvas)),
                Gestures{}
                    .onTouchDown([this](Point pos) {
                        // std::cout << "X: " << pos.x << ", Y: " << pos.y << std::endl;
                        if (!isSelecting || isInControls(pos)) return;
                        setState([pos](auto& self) {
                            self.touchDownPoint = pos;
                        });
                    })
                    .onTouchMove([this](Point pos) {
                        if (!isSelecting || isInControls(pos)) return;
                        setState([pos](auto& self) {
                            self.selectionWidth = abs(self.touchDownPoint.x - pos.x);
                            self.selectionHeight = abs(self.touchDownPoint.y - pos.y);
                            self.selectionStart = { std::min(self.touchDownPoint.x, pos.x),
                                std::min(self.touchDownPoint.y, pos.y) };
                        });
                    })
                );

            widgets.emplace_back(std::move(screenshot));
            if (isValidSelection()) {
                auto selection = Positioned(Border(Sized(Blank(), selectionWidth, selectionHeight),
                    Insets::all(2)), selectionStart);
                widgets.emplace_back(selection);
            }
            widgets.emplace_back(std::move(screenshot_controls(context)));
            if (!isSelecting) widgets.emplace_back(std::move(copy_dialog(context)));

            return Stack(std::move(widgets), !isSelecting, isSelecting);
        }

        auto build(rmlib::AppContext& context,
             const rmlib::BuildContext& buildCtx) const {
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
        bool isInControls(rmlib::Point pos) const;
        std::string buildCopyCommand(std::string quality, std::string mode) const;

        rmlib::Point touchDownPoint = {0, 0};
        rmlib::Point selectionStart = {0, 0};
        int selectionWidth = 0;
        int selectionHeight = 0;
        bool isSelecting = true;

        std::string quality;
        std::string mode;
};
