set echo on
echo "Building assemblies..."

cFilenames="/home/petermiller/Desktop/Handmade-Kohi/engine/application.cpp"
# echo "Files:" $cFilenames

pchnames=$(find . -type f -name "*.h")

assembly="engine"
compilerFlags="-g -shared -fdeclspec -fPIC -Werror=vla"
# -fms-extensions 
# -Wall -Werror
includeFlags="-Isrc -I$VULKAN_SDK/include"
linkerFlags="-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -L$VULKAN_SDK/lib -L/usr/X11R6/lib"
defines="-D_DEBUG -DKEXPORT -D_KUSE_SIMD"

echo "Building $assembly..."
cd bin
echo clang $cFilenames $compilerFlags -o lib$assembly.so $defines $includeFlags $linkerFlags 
clang $cFilenames $compilerFlags -o lib$assembly.so $defines $includeFlags $linkerFlags 
cd ..
cFilenames="/home/petermiller/Desktop/Handmade-Kohi/engine/entry.cpp"
# echo "Files:" $cFilenames

assembly="testbed"
compilerFlags="-g -fdeclspec -fPIC" 
# -fms-extensions 
# -Wall -Werror
defines="-D_DEBUG -DKIMPORT -D_KUSE_SIMD"

linkerFlags="-L/home/petermiller/Desktop/Handmade-Kohi/bin/ -lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -L$VULKAN_SDK/lib -L/usr/X11R6/lib -lengine -Wl,-rpath,."

echo "Building $assembly..."
cd bin
echo clang $cFilenames $compilerFlags -o $assembly $defines $includeFlags $linkerFlags
clang $cFilenames $compilerFlags -o $assembly $defines $includeFlags $linkerFlags
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

echo "All assemblies built successfully."

./testbed
cd ..
