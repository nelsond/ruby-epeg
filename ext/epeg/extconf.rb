require "mkmf"

INCLUDE_DIRS = [ RbConfig::CONFIG["includedir"] ]
LIB_DIRS     = [ RbConfig::CONFIG["libdir"] ]

dir_config("jpeg", INCLUDE_DIRS, LIB_DIRS)
dir_config("exif", INCLUDE_DIRS, LIB_DIRS)

raise "Please install libjpeg." unless have_library("jpeg")
raise "Please install libexif." unless have_library("exif")

create_makefile("epeg")
