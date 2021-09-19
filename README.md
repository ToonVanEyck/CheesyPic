# Cheeyspic PhotoBooth
The cheesypic software transforms your rasberry-pi in a full fledged photobooth. The software will work with DSLR cameras and dye-sub photo printers to deliver a high quality photobooth experience.

## Features
- Live view
- Custom photo strip designs
- Local storage of captured photos
- Custom themes
- Samba file server
  
## Supported Devices
### Cameras
The cameras is interfaced using libgphoto2. In theory the software will support any camera that libgphoto2 support.

[Supported Cameras](http://gphoto.org/proj/libgphoto2/support.php)
### Printers
The printer is interfaced using guttenprint and Solomon Peachy's selphy_print sofware.

[Supported dye-sub printers](https://www.peachyphotos.com/blog/stories/dye-sublimation-photo-printers-and-linux/)

## Required packages
install with ```sudo apt install```
- cmake
- cups
- libgphoto2-dev
- libturbojpeg0-dev 
- libxml2-dev 
- libjson-c-dev 
- libglib2.0-dev
- libcairo2-dev 
- libgdk-pixbuf2.0-dev 
- librsvg2-dev 
- libb64-dev 
- libgtk-3-dev
- libusb-1.0-0-dev
- xorg-dev
- mesa-utils
- libcups2-dev

## Installation

## Auto start

----------------------------


```bash
sudo apt install cmake
sudo apt install libgphoto2-dev
sudo apt install libturbojpeg0-dev 
sudo apt install libxml2-dev 
sudo apt install libjson-c-dev 
sudo apt install libglib2.0-dev
sudo apt install libcairo2-dev 
sudo apt install libgdk-pixbuf2.0-dev 
sudo apt install librsvg2-dev 
sudo apt install libb64-dev 
sudo apt install libgtk-3-dev
sudo apt install libusb-1.0-0-dev
sudo apt install xorg-dev #maybe

sudo apt install mesa-utils

#install cups
sudo apt install cups
sudo apt install libcups2-dev
#mabe needed
wget https://git.shaftnet.org/cgit/selphy_print.git/snapshot/selphy_print-gutenprint_5.3.4.tar.gz
tar -xf gutenprint-5.3.4.tar.xz
cd gutenprint-5.3.4
./configure
make
sudo make install
cd..
#deff needed
git clone https://git.shaftnet.org/cgit/selphy_print.git/
cd selphy_print
make
sudo make install
cd lib70x
make
sudo make install
sudo bash
echo '/usr/local/lib' >> /etc/ld.so.conf.d/local.conf
ldconfig
exit
cd ..
# misc
git clone https://github.com/kbranigan/Simple-OpenGL-Image-Library
git clone https://github.com/glfw/glfw
```

Note:
add the user to the lp group
`sudo adduser $USER lp

Setup of button DTO 

```bash
sudo dtc -I dts -O dtb -o /boot/overlays/photobooth_button.dtbo photobooth_button.dts
```
inally the following line must be added to /boot/config.txt:
dtoverlay=photobooth_button

https://kernelmastery.com/enable-regular-users-to-add-printers-to-cups/

disable gvfs-gphoto:
sudo systemctl mask gvfs-daemon
systemctl --user mask gvfs-daemon

Automatic boot:
---------------
https://die-antwort.eu/techblog/2017-12-setup-raspberry-pi-for-kiosk-mode/







commands | Discription
---------|---------------------------------
'c'      | start capture
'a'      | toggle active / inactive logic
'+'/'-'  | cycle states
'l'      | toggle mirror liveview
'm'      | toggle mirror reveal
'f'      | toggle fast mode
'w'      | toggle windowed / fulscreen mode
's'      | exit but wait 15s before stopping