#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd ${DIR}

function corecount {
    getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu
}

source build.options
source install.options

if [ "$(uname)" == "Darwin" ]; then
    DTAGS="-headerpad_max_install_names"
else
    DTAGS="--enable-new-dtags"
fi

export CC=${CC}
export LIB_C_FLAGS="-shared -fPIC -ldl -lm -lpthread"
export DRIVER_C_FLAGS="-ldl -lm -lpthread -Wl,${DTAGS},-rpath,${lib_dir}"
export _YEDVER_C_FLAGS="-lyed -Wl,${DTAGS},-rpath,${lib_dir}"
export PLUGIN_C_FLAGS="-shared -fPIC -I${DIR}/include -L${DIR}/lib -lyed"

strnstr_test_prg="#include <string.h>\nint main() { strnstr(\"haystack\", \"needle\", 8); return 0; }"
if ! echo -e "${strnstr_test_prg}" | cc -Wall -x c -o /dev/null > /dev/null 2>&1 -; then
    LIB_C_FLAGS+=" -DNEED_STRNSTR"
fi

# Add this framework to the Mac debug build so
# that we can use Instruments.app to profile yed
if [ "$(uname)" == "Darwin" ]; then
    debug+=" -framework CoreFoundation"
fi

if [ "$#" -ge 1 ]; then
    cfg=${!1}
else
    cfg=${release}
fi

# I hate getting spammed with error messages
ERR_LIM=" -fmax-errors=3 -Wno-unused-command-line-argument"

${CC} --version ${ERR_LIM} 2>&1 > /dev/null

if [ $? -eq 0 ]; then
    cfg+=${ERR_LIM}
fi

export cfg=${cfg}

mkdir -p lib
make all -j$(corecount) || exit 1

if [ "${strip}x" = "yesx" ]; then
    echo "Stripping binaries..."
    for f in $(find . -type f -name "*.so"); do
        strip "$f" || exit 1
    done

    strip libyed.so || exit 1
    strip _yed || exit 1
fi
