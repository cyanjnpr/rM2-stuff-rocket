#pragma once

#include <UI/Gesture.h>
#include <UI/StatefulWidget.h>

namespace rmlib {

class Button : public StatefulWidget<Button> {
  class State : public StateBase<Button> {
  public:
    auto build(AppContext& ctx, const BuildContext& buildCtx) const {
      return GestureDetector(Padding(Border(Border(Padding(Text(getWidget().text, getWidget().fontSize), 
                                                   Insets::all(getWidget().internalPadding)),
                                                Insets::all(2),
                                                down ? black : white),
                                            Insets::all(2)),
                                     Insets::all(2)),
                             Gestures{}
                               .onTouchDown([this](auto pos) {
                                  if (!getWidget().clickable) return;
                                 setState([](auto& self) { self.down = true; });
                               })
                               .onTap([this] {
                                  if (!getWidget().clickable) return;
                                  setState(
                                    [](auto& self) { self.down = false; });
                                  getWidget().onClick();
                               }));
    }

  int getWidgetSize() {
    return 2 + 2 + 2 + getWidget().internalPadding + getWidget().fontSize;
  }

  private:
    bool down = false;
  };

public:
  Button(std::string text, Callback onClick, int fontSize = default_text_size, int internalPadding = 5, bool clickable = true)
    : text(std::move(text)), onClick(std::move(onClick)), fontSize(fontSize), 
      internalPadding(internalPadding), clickable(clickable) {}

  static State createState() { return State{}; }

private:
  std::string text;
  Callback onClick;
  int fontSize;
  int internalPadding;
  bool clickable;
};

class RoundButton : public StatefulWidget<RoundButton> {
  class State : public StateBase<RoundButton> {
  public:
    auto build(AppContext& ctx, const BuildContext& buildCtx) const {
      return GestureDetector(Padding(RoundBorder(RoundBorder(Padding(Text(getWidget().text, getWidget().fontSize), 
                                                   Insets::all(getWidget().internalPadding)),
                                                Insets::all(2),
                                                down ? black : white),
                                            Insets::all(2)),
                                     Insets::all(2)),
                             Gestures{}
                               .onTouchDown([this](auto pos) {
                                  if (!getWidget().clickable) return;
                                 setState([](auto& self) { self.down = true; });
                               })
                               .onTap([this] {
                                  if (!getWidget().clickable) return;
                                  setState(
                                    [](auto& self) { self.down = false; });
                                  getWidget().onClick();
                               }));
    }

  int getWidgetSize() {
    return 2 + 2 + 2 + getWidget().internalPadding + getWidget().fontSize;
  }

  private:
    bool down = false;
  };

public:
  RoundButton(std::string text, Callback onClick, int fontSize = default_text_size, int internalPadding = 5, bool clickable = true)
    : text(std::move(text)), onClick(std::move(onClick)), fontSize(fontSize), 
      internalPadding(internalPadding), clickable(clickable) {}

  static State createState() { return State{}; }

private:
  std::string text;
  Callback onClick;
  int fontSize;
  int internalPadding;
  bool clickable;
};

} // namespace rmlib
