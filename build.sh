#!/bin/bash

scriptDir="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
buildDir="$scriptDir/build"
cmake "-H$scriptDir" "-B$buildDir"
cmake --build "$buildDir" --config Debug
