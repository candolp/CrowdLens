echo  "Building and runin main.cpp"

#export PICO_SDK_PATH=/home/candolp/DEV/pico-sdk
#echo "path to sdk is ${PICO_SDK_PATH}"
echo "cloning application ..."
git clone https://github.com/candolp/CrowdLens.git

echos "install dependencies ..."
sudo apt update
sudo apt install cmake build-essential libopencv-dev libcurl4-openssl-dev git libcamera-dev
echo "build application ... "
rm -rf build
mkdir build
cd ./build
cmake ..
make
echo "running Croud lens"
./src/demo/CrowdLens
#./blink
#./find_line_by_name
