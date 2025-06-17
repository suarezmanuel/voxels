set -e # if a command fails, exit immediatly

if [ -z "$1" ]; then
    echo "missing flag parameter, e.g. ./rebuild.bash {MACHINE}\n'MACHINE' = MAC, UNIX"
    exit 1
fi

rm -rf build
mkdir build
cd build 
cmake ..
cmake --build .
echo $1
cd ..

if [ $1 == "MAC" ]; then
    open VoxelCube.app
elif [ $1 == "UNIX" ]; then
    ./build/VoxelCube
fi
