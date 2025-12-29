#!/bin/bash
# Usage: ./FullscreenHandler.sh <address> <fullscreen> <workspace>
ADDR="$1"
FS="$2"          # 0, 1, or 2
WS="$3"          # 1-10, or -1 to reset all

# Config file to edit
HYPR_CONF="$HOME/.config/hypr/UserConfigs/UserDecorations.conf"  # adjust if needed

# Normal vs Fullscreen configuration
NO_BORDER_GAP="gapsin:0, gapsout:0"
NORMAL_BORDER_GAP="gapsin:3, gapsout:2"

if [ "$WS" -eq -1 ]; then
    for i in {1..10}; do
        LINE_TO_INSERT="workspace = ${i}, $NORMAL_BORDER_GAP"
        sed -i "/^#${i}:DYNAMIC WORKSPACE PLACEHOLDER \[ns\]/{n;s/.*/$LINE_TO_INSERT/;}" "$HYPR_CONF"
    done
    #echo "Reset all workspaces to normal padding"
    exit 0
fi

# 0 = not fs, 1 = fs, 2 = exclusive fs
if [ "$FS" -eq 1 ]; then
    LINE_TO_INSERT="workspace = ${WS}, $NO_BORDER_GAP"
else
    LINE_TO_INSERT="workspace = ${WS}, $NORMAL_BORDER_GAP"
fi

# Use sed to replace the line after the workspace comment, in-place
sed -i "/^#${WS}:DYNAMIC WORKSPACE PLACEHOLDER \[ns\]/{n;s/.*/$LINE_TO_INSERT/;}" "$HYPR_CONF"
#echo "Updated workspace $WS with $( [ $FS -eq 1 ] && echo 'no-border padding' || echo 'normal padding')"