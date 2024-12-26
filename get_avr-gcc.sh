#!/bin/sh
echo ""
echo "    Other releases can be found:"
echo "    https://github.com/ZakKemble/avr-gcc-build/releases"
echo ""
# I am use 8.3.0 version for linux
echo "  Download avr-gcc-8.3.0-x64-linux.tar.bz2"
wget https://github.com/ZakKemble/avr-gcc-build/releases/download/v8.3.0-1/avr-gcc-8.3.0-x64-linux.tar.bz2
echo "  Extract to ./avr-gcc-8.3.0-x64-linux/"
echo ""
tar xf avr-gcc-8.3.0-x64-linux.tar.bz2
cd ./avr-gcc-8.3.0-x64-linux/
./permissions.sh
echo "  Done!"
echo ""
