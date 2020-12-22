QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bvh_node.cpp \
    constant_medium.cpp \
    hittable_list.cpp \
    main.cpp \
    mainwindow.cpp \
    moving_sphere.cpp \
    objloader.cpp \
    rotate.cpp \
    sphere.cpp \
    triangle.cpp \
    two_axis_rect.cpp \
    world.cpp

HEADERS += \
    aabb.h \
    bvh_node.h \
    camera.h \
    constant_medium.h \
    cube.h \
    flip_face.h \
    hittable.h \
    hittable_list.h \
    mainwindow.h \
    material.h \
    moving_sphere.h \
    objloader.h \
    onb.h \
    perlin.h \
    random_generator.h \
    ray.h \
    rotate.h \
    sphere.h \
    texture.h \
    translate.h \
    triangle.h \
    two_axis_rect.h \
    world.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
