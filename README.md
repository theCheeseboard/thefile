# theFile
Linux File Manager based on Qt

## Screenshots
![Screenshot 1](https://raw.githubusercontent.com/vicr123/thefile/master/images/screenshot1.png)

## Dependencies
- qmake
- clang
- qt5-base
- kwidgetsaddons
- xdg-utils
- jmtpfs (optional, if you want to access MTP devices)
- ifuse (optional, if you want to access iOS devices)
- usbmuxd (optional, if you want to access iOS devices)
- libplist (optional, if you want to access iOS devices)

## Build
```
qmake
make
```

## Install
1. Copy thefile over to /usr/bin
2. Copy thefile.desktop over to /usr/share/applications

## Packages
theFile is available in the Arch Linux AUR.

## Warnings
- iOS Support is experimental.
