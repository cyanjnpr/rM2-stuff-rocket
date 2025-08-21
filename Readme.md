reMarkable Stuff (fork)
================

Collection of reMarkable related apps, utilities and libraries.

Differences compared to the upstream
--------

### Rocket

Launcher that uses the power button to show.

Changes made in this fork:
- If set, passcode is copied from xochitl to display additional lockscreen in rocket itself
    - instead of timeout, after multiple failures to authenticate, device is shutdown to maintain security
- Launcher keeps focus on the previous app not the current one
    - this behavior enables switching between two currently used apps by double clicking the power button
- Standard suspended.png is displayed on suspend
- Poweroff & reboot buttons in the launcher menu
- **Very** early progress on allowing screenshots to be copied into xochitl notebooks
    - you need to install [karmtka](https://github.com/cyanjnpr/karMtka) first and place it in a directory added to the PATH
    - long press the power button to take a screenshot
    - select area which you intend to copy
    - press "Copy to Xochitl" button to copy into current notebook (overwrites current page)
    - in the current state this **will** damage your notebooks, it's all WIP

<img src="doc/rocket_lockscreen.png" width=500/>
<img src="doc/rocket_menu.png" width=500/>

When pressing the power button, the app drawer will be shown with a timeout of 10 seconds.
After 10 seconds the device will go to sleep, unless an app is picked before that timeout.
You can also interrupt the timeout by pressing the `[x]` button.

This allows you to switch apps without relying on gestures.

### rMlib

Library for writing remarkable apps.
Includes an extensive declarative UI framework based on Flutter.

Changes made in this fork:
- a few tiny changes to widgets and layouts to improve look & feel

Building
--------

Building for the remarkable can either use the [toltec toolchain](https://github.com/toltec-dev/toolchain)
or the reMarkable one:
```bash
# For toltec:
$ cmake --preset dev-toltec
# For remarkable:
$ cmake --preset dev

# To build everything:
$ cmake --build build/dev
# Or to build a specific app:
$ cmake --build build/dev --target yaft

# To create an ipk file:
$ cmake --build build/dev --target package
```

Emulating
---------

For faster development an `EMULATE` mode is supported by rMlib. This allows
running most apps on a desktop using SDL to emulate the remarkable screen.
To enable it pass `-DEMULATE=ON` to the cmake configure command, without using
the reMarkable toolchain of course.
```bash
$ cmake --preset dev-host
$ cmake --build build/host --target yaft
$ ./build/host/apps/yaft/yaft # Should launch Yaft with an emulated screen in a separete window.
```
