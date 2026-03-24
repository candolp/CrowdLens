echo  "Building and runin main.cpp"

#export PICO_SDK_PATH=/home/candolp/DEV/pico-sdk
#echo "path to sdk is ${PICO_SDK_PATH}"
rm -rf build
mkdir build
cd ./build
cmake ..
make
./src/demo/CrowdLens
#./blink
#./find_line_by_name
