cd subprojects/

# Build libtdl
git clone --depth 1 git://git.savannah.gnu.org/libtool.git
mv libtool/libltdl .
rm -rf libtool
cd libltdl
./configure --enable-ltdl-install --host=arm-linux-gnueabihf --prefix=$(pwd)
make
make install

# Move libltdl into ligphoto2
mv lib/* ../libgphoto2 

# Build libusb
cd ../
git clone --depth 1 https://github.com/libusb/libusb.git
cd libusb/
./configure --host=arm-linux-gnueabihf --prefix=$(pwd) --disable-udev
make
make install

# Move libusb pkgconfig into subprojects 
mv lib/pkgconfig/* ..

# Build libgphoto2
cd ../libgphoto2
autoreconf -ivf
./configure --host=arm-linux-gnueabihf --prefix=$(pwd) LDFLAGS=-L$(pwd) --without-libxml-2.0
make
make install
mv libgphoto2.pc ..
mv libgphoto2_port.pc ..
