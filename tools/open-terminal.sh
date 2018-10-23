#!/bin/bash
set -e

if [ $# -eq 0 ]; then
    CMD="bash"
else
    CMD="$1"
fi
echo "Running ${CMD}"

run_on_osx_term() {
    osascript <<EOF
    tell application "Terminal"
        do script "bash -c '${CMD}; exit $?'"
        activate
    end tell
EOF
}

run_on_osx_iterm() {
    osascript <<EOF
    tell application "iTerm2"
        create window with default profile command "${CMD}"
        activate
    end tell
EOF
}

if [ `uname -s` == "Darwin" ]; then
    if [ -d /Applications/iTerm.app ]; then
        run_on_osx_iterm
    else
        run_on_osx_term
    fi
else
    echo "Unsupported environment"
    exit 1
fi
exit 0