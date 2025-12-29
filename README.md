# hypr-fullscreen-window-watcher
Watches fullscreen window status and can run arbitrary scripts / commands with window details. 0% CPU usage at idle.

## Why
You can do something like `windowrule = rounding 0, fullscreen:1`, but you can't dynamically adjust the window gaps. If you like nice gaps between your tiled windows but you ALSO want a window to sit FLUSH with absolutely no gap in non-exclusive fullscreen, you cannot achieve that in native Hyprland currently.

Unless we just dynamically modify the Hyprland config depending on fullscreen state.

This can work because if a window is fullscreen, it's already the only window you can see on a workspace, so editing a per workspace config is almost like a per-fullscreen-window config.

## Install
- `gcc fullscreen-window-watcher.c -o fullscreen-window-watcher`
- `mv ./fullscreen-window-watcher /usr/local/bin/`

## Setup
- fullscreen-window-watcher tags (and untags) the window `fullscreen_mode` for any other future actions and runs a script (`FullscreenHandler.sh`) every time it detects a fullscreen change. 
- FullscreenHandler.sh knows the workspace from input, so can use sed to replace the configuration line in your Hyprland config. 
- Tested on 0.52.1, it takes effect immediately without any `hyprctl reload`

## Startup
`exec-once = /usr/local/bin/fullscreen-window-watcher` in your preferred Hyprland startup conf