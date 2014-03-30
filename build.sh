#!/bin/bash

ARCH=$1
BUILDTYPE=$2

if [ "x$ARCH" == "x" ]; then 
    ARCH=32
fi

if [ "x$BUILDTYPE" == "x" ]; then 
    BUILDTYPE=Release
fi

case "$ARCH" in
"32")
    TARGET=package
    ;;
"64")
    TARGET=package
    ;;
"android-arm")
    CMAKEFLAGS="-DCMAKE_TOOLCHAIN_FILE=../../../tools/cmake/toolchains/android.toolchain.cmake -DANDROID_ABI=armeabi"
    ;;
"android-armv7a")
    CMAKEFLAGS="-DCMAKE_TOOLCHAIN_FILE=../../../tools/cmake/toolchains/android.toolchain.cmake -DANDROID_ABI=armeabi-v7a"
    ;;
"android-mips")
    CMAKEFLAGS="-DCMAKE_TOOLCHAIN_FILE=../../../tools/cmake/toolchains/android.toolchain.cmake -DANDROID_ABI=mips"
    ;;
"android-x86")
    CMAKEFLAGS="-DCMAKE_TOOLCHAIN_FILE=../../../tools/cmake/toolchains/android.toolchain.cmake -DANDROID_ABI=x86"
    ;;
 
*)
    echo "Platform $ARCH unsupported."
    exit 1
    ;;
esac


CWD=`pwd`
mkdir -p build/$ARCH/$BUILDTYPE && cd build/$ARCH/$BUILDTYPE && cmake -DARCH=$ARCH $CMAKEFLAGS -DCMAKE_BUILD_TYPE=$BUILDTYPE -G "Eclipse CDT4 - Unix Makefiles" ../../..

if [ "$?" != "0" ]; then 
    echo CMAKE failed.
    exit 1
fi

cd $CWD
make -C build/$ARCH/$BUILDTYPE  $TARGET
