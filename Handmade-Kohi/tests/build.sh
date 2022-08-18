#!/bin/bash
# Build script for testbed
set echo on

# Get a list of all the .c files.
#//cFilenames="main.cpp"
cFilenames=$(find . -type f -name "*.cpp")

# echo "Files:" $cFilenames

assembly="tests"
compilerFlags="-g -fdeclspec -fPIC" 
# -fms-extensions 
# -Wall -Werror
includeFlags="-Isrc -I../engine/"
linkerFlags="-L../bin/ -lengine -Wl,-rpath,."
defines="-D_DEBUG -DKIMPORT"

echo "Building $assembly..."
echo clang $cFilenames $compilerFlags -o ../bin/$assembly $defines $includeFlags $linkerFlags
clang $cFilenames $compilerFlags -o ../bin/$assembly $defines $includeFlags $linkerFlags
cd ..
cd bin
./tests
cd ..

