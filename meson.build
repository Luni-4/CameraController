project('camera_controller', 'cpp')

incdirs = [include_directories('src'), include_directories('src/camera'), include_directories('src/wiringpi')]

src = [ 'src/main.cpp', 'src/camera/CameraWrapper.cpp', 
        'src/commands/Commands.cpp', 
        'src/communication/MessageDecoder.cpp', 'src/communication/MessageDecoder.cpp', 'src/communication/TCPServer.cpp',
        'src/functions/intervalometer.cpp', 'src/functions/sequencer.cpp',
        'src/utils/RemoteTrigger.cpp']

libsdir = meson.source_root() / 'libraries'

thread_dep = dependency('threads')
libexif = meson.get_compiler('cpp').find_library('exif', dirs: libsdir)
libgphoto2_port = meson.get_compiler('cpp').find_library('gphoto2_port', dirs: libsdir)
libgphoto2 = meson.get_compiler('cpp').find_library('gphoto2', dirs: libsdir)
libwiringpi = meson.get_compiler('cpp').find_library('wiringPi', dirs: libsdir)
libltdl = meson.get_compiler('cpp').find_library('ltdl', dirs: libsdir)

executable('cameracontroller', src, include_directories : incdirs, dependencies: [libexif, libgphoto2, libgphoto2_port, libwiringpi, libltdl, thread_dep])
