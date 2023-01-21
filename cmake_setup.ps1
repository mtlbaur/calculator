mkdir bin
cd bin
mkdir debug
mkdir release
cd ..
mkdir build
cd build
mkdir debug
mkdir release
cd debug
cmake -G 'Ninja' -S '../..' -DCMAKE_BUILD_TYPE=Debug -DCMAKE_RUNTIME_OUTPUT_DIRECTORY='../../bin/debug'
ninja # compile immediately
cd ..
cd release
cmake -G 'Ninja' -S '../..' -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY='../../bin/release'
ninja # compile immediately
cd '../..'