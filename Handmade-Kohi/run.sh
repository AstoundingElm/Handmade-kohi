set echo on
echo "Building assemblies..."

cFilenames="/home/petermiller/Desktop/Handmade-Kohi/engine/application.cpp"
# echo "Files:" $cFilenames

assembly="engine"
compilerFlags="-g -shared -fdeclspec -fPIC"
# -fms-extensions 
# -Wall -Werror
includeFlags="-Isrc -I$VULKAN_SDK/include"
linkerFlags="-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -L$VULKAN_SDK/lib -L/usr/X11R6/lib"
defines="-D_DEBUG -DKEXPORT"

echo "Building $assembly..."
cd bin
clang $cFilenames $compilerFlags -o lib$assembly.so $defines $includeFlags $linkerFlags
cd ..
cFilenames="/home/petermiller/Desktop/Handmade-Kohi/engine/entry.h"
# echo "Files:" $cFilenames

assembly="testbed"
compilerFlags="-g -fdeclspec -fPIC" 
# -fms-extensions 
# -Wall -Werror
defines="-D_DEBUG -DKIMPORT"

echo "Building $assembly..."
cd bin
echo clang $cFilenames $compilerFlags -o $assembly $defines $includeFlags $linkerFlags

ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

echo "All assemblies built successfully."

./testbed
cd ..
