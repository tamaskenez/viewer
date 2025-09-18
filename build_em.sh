#!/bin/bash -e

if [[ ! -z "$1" ]]; then
	build_type_option=-DCMAKE_BUILD_TYPE=$1
	emcmake cmake bw $build_type_option
fi

cmake --build bw -j
emrun --browser chrome ./bw/src/viewer/viewer.html
