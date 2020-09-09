required packages 

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

#install cups
sudo apt install cups
sudo apt install libcups2-dev
wget https://sourceforge.net/projects/gimp-print/files/gutenprint-5.3/5.3.3/gutenprint-5.3.3.tar.xz
tar -xf gutenprint-5.3.3.tar.xz
#./configure --> make --> make install



git clone git://git.shaftnet.org/selphy_print.git
git clone https://github.com/kbranigan/Simple-OpenGL-Image-Library
git clone https://github.com/glfw/glfw
```

Note:
add the user to the lp group
`sudo adduser $USER lp`

https://kernelmastery.com/enable-regular-users-to-add-printers-to-cups/

disable gvfs-gphoto:
systemctl --user stop gvfs-daemon
systemctl --user mask gvfs-daemon


commands:
'c'     start capture
'a'     toggle active / inactive logic
'+'/'-' cycle states
'l'     toggle mirror liveview
'm'     toggle mirror reveal
'f'     toggle fast mode
'w'     toggle windowed / fulscreen mode