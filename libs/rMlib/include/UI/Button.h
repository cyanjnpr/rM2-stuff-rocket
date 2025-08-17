#pragma once

#include <UI/Gesture.h>
#include <UI/StatefulWidget.h>

namespace rmlib {

class Button : public StatefulWidget<Button> {
  class State : public StateBase<Button> {
  public:
    auto build(AppContext& ctx, const BuildContext& buildCtx) const {
      return GestureDetector(Padding(Border(Border(Text(getWidget().text, getWidget().fontSize),
                                                   Insets::all(getWidget().internalPadding),
                                                   down ? black : white),
                                            Insets::all(2)),
                                     Insets::all(2)),
                             Gestures{}
                               .onTouchDown([this](auto pos) {
                                 setState([](auto& self) { self.down = true; });
                               })
                               .onTap([this] {
                                 setState(
                                   [](auto& self) { self.down = false; });
                                 getWidget().onClick();
                               }));
    }

  private:
    bool down = false;
  };

public:
  Button(std::string text, Callback onClick, int fontSize = default_text_size, int internalPadding = 5)
    : text(std::move(text)), onClick(std::move(onClick)), fontSize(fontSize), internalPadding(internalPadding) {}

  static State createState() { return State{}; }

private:
  std::string text;
  Callback onClick;
  int fontSize;
  int internalPadding;
};

} // namespace rmlib
