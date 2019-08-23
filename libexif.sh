cd subprojects/libexif
autoreconf -ivf
./configure --host=arm-linux-gnueabihf --prefix=$(pwd)
make
make install
mv libexif.pc ..
