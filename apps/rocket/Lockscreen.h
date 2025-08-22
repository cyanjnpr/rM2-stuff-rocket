#pragma once

#include <unistdpp/file.h>
#include <UI.h>

namespace {
    const std::string xochitl_conf_path = "/home/root/.config/remarkable/xochitl.conf";
}

class LockscreenState;

class LockscreenWidget : public rmlib::StatefulWidget<LockscreenWidget> {
    public:
        LockscreenWidget(rmlib::Callback onUnlock) 
            : onUnlock(std::move(onUnlock)) {
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

            if (xochitlPasscode.empty()) {
                this->onUnlock();
            }
        }

        static LockscreenState createState();

        rmlib::Callback onUnlock;
        std::string xochitlPasscode = "";
};

class LockscreenState : public rmlib::StateBase<LockscreenWidget> {

    public:
        void init(rmlib::AppContext& context, const rmlib::BuildContext&/*unused*/);

        auto lockscreen(rmlib::AppContext& context) const {
            using namespace rmlib;
            
            std::string passcodeCopy = passcode;
            std::fill(passcodeCopy.begin(), passcodeCopy.end(), '*');

            auto unlockText = Text("Enter passcode", 1.5 * default_text_size);
            auto passcodeText = Text(passcodeCopy, 1.5 * default_text_size);
            
            auto b1 = RoundButton(
            "1", [this] { setState([](auto& self) { self.typePasscode('1'); }); }, 
            default_text_size * 1.5, 25);
            auto b2 = RoundButton(
            "2", [this] { setState([](auto& self) { self.typePasscode('2'); }); }, 
            default_text_size * 1.5, 25);
            auto b3 = RoundButton(
            "3", [this] { setState([](auto& self) { self.typePasscode('3');}); }, 
            default_text_size * 1.5, 25);
            auto b4 = RoundButton(
            "4", [this] { setState([](auto& self) { self.typePasscode('4'); }); }, 
            default_text_size * 1.5, 25);
            auto b5 = RoundButton(
            "5", [this] { setState([](auto& self) { self.typePasscode('5'); }); }, 
            default_text_size * 1.5, 25);
            auto b6 = RoundButton(
            "6", [this] { setState([](auto& self) { self.typePasscode('6'); }); }, 
            default_text_size * 1.5, 25);
            auto b7 = RoundButton(
            "7", [this] { setState([](auto& self) { self.typePasscode('7'); }); }, 
            default_text_size * 1.5, 25);
            auto b8 = RoundButton(
            "8", [this] { setState([](auto& self) { self.typePasscode('8'); }); }, 
            default_text_size * 1.5, 25);
            auto b9 = RoundButton(
            "9", [this] { setState([](auto& self) { self.typePasscode('9'); }); }, 
            default_text_size * 1.5, 25);
            auto b0 = RoundButton(
            "0", [this] { setState([](auto& self) { self.typePasscode('0'); }); }, 
            default_text_size * 1.5, 25);
            auto bClear = RoundButton(
            "<=", [this] { setState([](auto& self) { 
                if (!self.passcode.empty()) {
                self.passcode.pop_back();
            } }); }, 
            default_text_size * 0.75, 25);

            return Cleared(Column(
            Padding(unlockText, Insets::all(50)),
            Padding(passcodeText, Insets::all(100)), 
            Row(Padding(b1, Insets::all(30)), Padding(b2, Insets::all(30)), Padding(b3, Insets::all(30))),
            Row(Padding(b4, Insets::all(30)), Padding(b5, Insets::all(30)), Padding(b6, Insets::all(30))),
            Row(Padding(b7, Insets::all(30)), Padding(b8, Insets::all(30)), Padding(b9, Insets::all(30))),
            Row(Padding(b0, Insets::sides(30, 30, 116 + default_text_size * 1.5, 30)), Padding(bClear, Insets::all(30)))
            // 116 = 30 * 2 + 2 * 3 + 25 * 2
            ));
        }

        auto build(rmlib::AppContext& context,
             const rmlib::BuildContext& /*unused*/) const {
            using namespace rmlib;

            auto ui = [&]() -> DynamicWidget {
                return lockscreen(context);
            }();

            return ui;
        }

    private:
        void typePasscode(char rune);

        bool isPasscodeGood = false;
        std::string passcode;
        int attempts;
};
