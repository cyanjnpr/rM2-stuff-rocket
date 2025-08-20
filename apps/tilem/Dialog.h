#pragma once

#include <UI.h>
#include <UI/Navigator.h>

class DownloadDialog : public rmlib::StatefulWidget<DownloadDialog> {
public:
  class State : public rmlib::StateBase<DownloadDialog> {
  public:
    void init(rmlib::AppContext& ctx, const rmlib::BuildContext& buildCtx);

    auto build(rmlib::AppContext& appCtx,
               const rmlib::BuildContext& ctx) const {
      using namespace rmlib;

      return Center(Border(
        Cleared(Text("Downloading ROM '" + getWidget().romPath + "' ...")),
        Insets::all(5)));
    }

  private:
    rmlib::TimerHandle startTimer;
  };

  DownloadDialog(std::string_view romPath) : romPath(romPath) {}

  static State createState();

  std::string romPath;
};

class ErrorDialog : public rmlib::StatelessWidget<ErrorDialog> {
public:
  ErrorDialog(std::string_view romPath) : romPath(romPath) {}

  auto build(rmlib::AppContext& appCtx, const rmlib::BuildContext& ctx) const {
    using namespace rmlib;
    return Center((Border(
      Cleared(Column(
        // i don't use wifi on my rM
        Text("Loading ROM '" + romPath + "' failed. Download it and restart the app."),
        Row(
          // Padding(Button("Download", [&ctx] { Navigator::of(ctx).pop(); }),
          //           Insets::all(10)),
            Padding(Button("Exit", [&appCtx] { appCtx.stop(); }),
                    Insets::all(10))))),
      Insets::all(5))));
  }
  std::string romPath;
};
