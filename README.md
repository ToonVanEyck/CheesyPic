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
### Cheesypic
```bash
cd cheesypic
mkdir build
cd build
cmake ..
make
sudo make install
```

### Guttenprint / Selphy-print
```bash
# Gain root
sudo su -
# Remove existing gutenprint packages
apt remove gutenprint*
# Install necessary development libraries
apt install libusb-1.0-0-dev libcups2-dev
# Download latest gutenprint snapshot from sourceforge
curl -o gutenprint-5.3.4-2021-08-18T01-00-2a241dff.tar.xz "https://master.dl.sourceforge.net/project/gimp-print/snapshots/gutenprint-5.3.4-2021-08-18T01-00-2a241dff.tar.xz?viasf=1"
# Decompress & Extract
tar -xJf gutenprint-5.3.4-2021-08-18T01-00-2a241dff.tar.xz
# Compile gutenprint
cd gutenprint-5.3.4-2021-08-18T01-00-2a241dff
./configure --without-doc
make -j4
make install
cd ..
# Get the latest selphy_print code
git clone git://git.shaftnet.org/selphy_print.git
# Compile selphy_print
cd selphy_print
make -j4 
make install
# Set up library include path
echo "/usr/local/lib" > /etc/ld.so.conf.d/usr-local.conf
ldconfig
# Refresh PPDs
cups-genppdupdate
# Restart CUPS
service cups restart 
# FiN
exit
```
Note: Some printers may require additional image processing library.

## GPIO trigger
The photobooth uses the 'c' key as a trigger to start the photobooth. A device-tree overlay can be added to configure gpio_1 as a keypad 'c' button.

Compile and install the *photobooth_button.dts* on the pi using:
```bash
sudo dtc -I dts -O dtb -o /boot/overlays/photobooth_button.dtbo misc/photobooth_button.dts
```
Connect a normaly open button between GPIO_1 and ground.

## disable gvfs-gphoto:
gvfs-gphoto automatically mounts cameras as a storage device on boot, this prevents cheesypic from using them. To disable gvfs run:
```bash
sudo systemctl mask gvfs-daemon
systemctl --user mask gvfs-daemon
```
Finally add the following line to ```/boot/config.txt```:
```bash
dtoverlay=photobooth_button
```

## Printer Setup
add the user to the lp group
```bash
sudo adduser $USER lp
```
Add your printer to cups using the cups webpage or lpadmin command.

Set the default printer:
```bash
lpoptions -d printer-name
```
Verify the default printer:
```bash
lpstat -d
```
## Auto start
The configuration described in this topic allow you to automatically start the cheesypic software on boot without a desktop environement.
### Openbox Configuration
edit ```/etc/xdg/openbox/autostart```:
```bash
# Disable any form of screen saver / screen blanking / power management
xset s off
xset s noblank
xset -dpms
# Allow quitting the X server with CTRL-ATL-Backspace
setxkbmap -option terminate:ctrl_alt_bksp
# Start Photobooth
cheesypic
```
### Start X on boot
edit ```.bash_profile```:
```bash
[[ -z $DISPLAY && $XDG_VTNR -eq 1 ]] && startx -- -nocursor
```
### Hide the boot terminal
edit ```/boot/cmdline.txt``` and add:
```bash
consoleblank=1 vt.global_cursor_default=0
```
### Hide the boot rainbow splash
edit ```/boot/config.txt``` and add:
```bash
disable_splash=1
```
## Reducing boot time
edit ```/boot/config.txt``` and add:
```bash
dtoverlay=pi3-disable-bt
boot_delay=0
```

## Usage / Dev commands

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
op
