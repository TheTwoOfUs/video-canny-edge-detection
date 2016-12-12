#!/usr/bin/bash

DISTRO=$(cat /etc/redhat-release | cut -d ' ' -f 1)

ERR_MSG="Unknown Linux distribution, aborting..."

UPSTREAM="https://github.com/iulianR/ffmpeg-shared-object-files"

declare -a LIBRARIES=("libavcodec.so" "libavcodec.so.57"
                      "libavformat.so" "libavformat.so.57"
                      "libavutil.so" "libavutil.so.55"
                      "libswresample.so" "libswresample.so.2"
                      "libswscale.so" "libswscale.so.4")


if [ -z "$DISTRO" ]; then
    echo -e "$ERR_MSG"
    exit 1
fi

if [ "$DISTRO" == "Fedora" ]; then
    sudo dnf install -y ffmpeg ffmpeg-libs ffmpeg-devel
elif [ "$DISTRO" == "CentOS" ]; then
    rm -rf libraries && mkdir libraries
    for lib in "${LIBRARIES[@]}"; do
        wget "$UPSTREAM"/blob/master/"$lib"?raw=true
        mv "$lib"?raw=true libraries/"$lib"
        chmod +x libraries/"$lib"
    done
else
    echo -e "$ERR_MSG"
    exit 1
fi
