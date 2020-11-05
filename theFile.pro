TEMPLATE = subdirs

libproj.subdir = libthefile

applicationproj.subdir = application
applicationproj.depends = libproj

SUBDIRS += \
    applicationproj \
    libproj
