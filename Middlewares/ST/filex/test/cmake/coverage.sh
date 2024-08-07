#!/bin/bash

set -e

cd $(dirname $0)
mkdir -p coverage_report/$1
gcovr --object-directory=build/$1/filex/CMakeFiles/filex.dir/common/src -r ../../common/src -e ".*driver.*" --xml-pretty --output coverage_report/$1.xml
gcovr --object-directory=build/$1/filex/CMakeFiles/filex.dir/common/src -r ../../common/src -e ".*driver.*" --html --html-details --output coverage_report/$1/index.html
