#!/usr/bin/bash

DISTRO=$(cat /etc/redhat-release | cut -d ' ' -f 1)
ERR_MSG="Unknown Linux distribution, aborting..."

declare -a LIBRARIES=("libavcodec.so" "libavcodec.so.57"
                      "libavformat.so" "libavformat.so.57"
                      "libavutil.so" "libavutil.so.55"
                      "libswresample.so" "libswresample.so.2"
                      "libswscale.so" "libswscale.so.4")


if [ -z "$DISTRO" ]; then
    echo -e "$ERR_MSG"
    exit 1
fi

if [ "$DISTRO" == "CentOS" ]; then
    # Download the FFmpeg source files.
    rm -rf ffmpeg
    wget https://github.com/FFmpeg/FFmpeg/archive/n3.2.zip && unzip n3.2.zip
    rm -f n3.2.zip && mv FFmpeg-n3.2 ffmpeg
    cd ffmpeg && ./configure --disable-yasm && cd ..

    # Download the pre-compiled shared object files.
    rm -rf libraries && mkdir libraries
    for lib in "${LIBRARIES[@]}"; do
        wget https://github.com/iulianR/ffmpeg-shared-object-files/blob/master/"$lib"?raw=true
        mv "$lib"?raw=true libraries/"$lib"
        chmod +x libraries/"$lib"
    done
else
    echo -e "$ERR_MSG"
    exit 1
fi
