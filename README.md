# CameraController

Remote control & Intervalometer for Nikon D3000 DSLR on a raspberry pi

## Install
To cross-compile this project, install the following packages:

    sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

## Cross-compilation

Issue the following commands:

    export PKG_CONFIG_PATH=/your_path_to_this_repo/subprojects
    meson build \
      --buildtype release \
      --cross-file cross_file.txt
    ninja -C build
