sudo mount -t cifs //192.168.0.13/other_remote_dev /home/candolp/DEV -o'user=kanop,pass=remoteUSER=-09',iocharset=utf8


#
#gpioget GPIO4
#
#
#gpioinfo GPIO4
#
#gpioget --numeric GPIO4
#
#gpiomon --num-events=3 --edges=rising GPIO3
#
#https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/snapshot/libgpiod-2.2.2.tar.gz
#tar -xvf ./libgpiod-2.2.2.tar.gz
#cd ./libgpiod-2.2.2/
#./configure --enable-tools


#install xrdp on debian

#sudo apt update
#sudo apt install -y xrdp
#sudo systemctl enable xrdp
#sudo systemctl start xrdp
#sudo systemctl status xrdp --no-pager

#
#install drivers and dependencies

sudo apt-get update
sudo apt-get install -y cmake build-essential libopencv-dev libcamera-dev libgpiod-dev

#make
#sudo make install