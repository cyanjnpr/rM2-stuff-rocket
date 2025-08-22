#pragma once

#include <algorithm>
#include <functional>

#include <UI/StatefulWidget.h>
#include <UI/Flex.h>
#include <UI/Button.h>
#include <UI/Text.h>

namespace rmlib {

    // SpinWidget from KOReader, but horizontal
    class SpinWidget : public StatefulWidget<SpinWidget> {
        class State : public StateBase<SpinWidget> {
        public:
            auto init(AppContext& ctx, const BuildContext& buildCtx) const {
                setState([](auto& self) { self.value = self.getWidget().min; });
            }

            auto build(AppContext& ctx, const BuildContext& buildCtx) const {
                int spacing = 10;

                return Row(Text(getWidget().label),
                    Padding(Column(
                        Button("<<=", [this]{
                            setState([](auto& self) { self.changeValue(-self.getWidget().jump); });
                        }),
                        Button(" <=", [this]{
                            setState([](auto& self) { self.changeValue(-self.getWidget().step); });
                        })
                    ), Insets::all(spacing)), 
                    Text(getWidget().transform(value)), 
                    Padding(Column(
                        Button("=>>", [this]{
                            setState([](auto& self) { self.changeValue(self.getWidget().jump); });
                        }),
                        Button("=> ", [this]{
                            setState([](auto& self) { self.changeValue(self.getWidget().step); });
                        })
                    ), Insets::all(spacing))
                );
            }

        private:
            void changeValue(int diff) {
                value = std::min(std::max(value + diff, getWidget().min), getWidget().max);
                getWidget().onUpdate(getWidget().transform(value));
            }

            int value = 0;
        };

        public:
        SpinWidget(std::string label, std::function<void (std::string val)> onUpdate, int min = 0, int max = 100, int step = 1, int jump= 10,
                std::function<std::string(int)> transform = [](int val) { return std::to_string(val); }) 
            : label(std::move(label)), min(min), max(max), step(step), jump(jump),
                transform(std::move(transform)), onUpdate(std::move(onUpdate)) {}

        static State createState() { return State{}; }

        private:
        int min;
        int max;
        int step;
        int jump; // hold_step in KOReader
        std::string label;
        std::function<std::string(int)> transform;
        std::function<void (std::string val)> onUpdate;
    };

}