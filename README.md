required packages 

```bash
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

git clone https://github.com/kbranigan/Simple-OpenGL-Image-Library
git clone https://github.com/glfw/glfw
```

Note:
add the user to the lp group
`sudo adduser $USER lp`
change permissons
`sudo chmod 777 /usr/lib/cups/backend/gutenprint53+usb`

commands:
'c'     start capture
'a'     toggle active / inactive logic
'+'/'-' cycle states
'l'     toggle mirror liveview
'm'     toggle mirror reveal
'f'     toggle fast mode