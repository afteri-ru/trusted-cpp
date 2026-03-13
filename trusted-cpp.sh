#!/bin/bash

SCRIPT_DIR="$(dirname -- ${BASH_SOURCE[0]})" # script directory
CLANG="${CLANG_TRUST:-clang-21}" # clang compiler clang-21 by default
PLUGIN="$SCRIPT_DIR/trusted-cpp_clang.so" # plugin library
CMD="-Xclang -load -Xclang $PLUGIN -Xclang -add-plugin -Xclang trust" # load plugin 

for arg in "$@"; do
    if [[ "$arg" == --trust* ]]; then
        arg="${arg#--trust}"   # remove prefix --trust
        CMD+=" -Xclang -plugin-arg-trust -Xclang $arg" # set argument to plugin
    fi
    CMD+=" $arg"
done

$CLANG -I$SCRIPT_DIR $CMD


