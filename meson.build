project(
    'camera_controller',
    'cpp',
    meson_version: '>= 0.51.0'
)

incdirs = [
    include_directories('src'),
    include_directories('src/camera'),
    include_directories('src/wiringpi')
]

src = [
    'src/main.cpp',
    'src/camera/CameraWrapper.cpp',
    'src/commands/Commands.cpp',
    'src/communication/MessageDecoder.cpp',
    'src/communication/MessageDecoder.cpp',
    'src/communication/TCPServer.cpp',
    'src/functions/intervalometer.cpp',
    'src/functions/sequencer.cpp',
    'src/utils/RemoteTrigger.cpp'
]

cpp = meson.get_compiler('cpp')
libsdir = meson.source_root() / 'libraries'

deps = []

# threads dependency
deps += dependency('threads')

# libefix dependency
libexif_dep = dependency('libexif', required : false)
if not libexif_dep.found()
    libexif_proj = subproject('libexif', required: false)
    if not libexif_proj.found()
        libexif_command = run_command('sh', 'libexif.sh')
        if libexif_command.returncode() != 0
           error(libexif_command.stderr().strip())
        endif
        libpath = meson.source_root() / 'subprojects' / 'libexif' / 'lib'
        libexif_dep = cpp.find_library('exif', dirs: libpath)
    endif
endif
deps += libexif_dep

deps += meson.get_compiler('cpp').find_library('gphoto2_port', dirs: libsdir)
deps += meson.get_compiler('cpp').find_library('gphoto2', dirs: libsdir)
deps += meson.get_compiler('cpp').find_library('wiringPi', dirs: libsdir)
deps += meson.get_compiler('cpp').find_library('ltdl', dirs: libsdir)

executable(
    'cameracontroller',
    src,
    include_directories : incdirs,
    dependencies: deps
)
