CONFIG += c++11 debug_and_release
TEMPLATE	= app
LANGUAGE	= C++
LIBS	+= -lboost_system -lpthread
QT += network widgets core
INCLUDEPATH	+= 	commands \
				../librender/src/r3c \
				../librender/src/jsonq
CONFIG += qt debug_and_release
DESTDIR = ../bin
CONFIG(debug, debug|release) {
	LIBS += -L../lib -lrenderd
	TARGET = render-clientd
	MOC_DIR = ../b/debug/render-client
	OBJECTS_DIR = ../b/debug/render-client
	UI_DIR = ../b/debug/render-client
	PRE_TARGETDEPS += ../lib/librenderd.a
} else {
	LIBS += -L../lib -lrender
	MOC_DIR = ../b/release/render-client
	OBJECTS_DIR = ../b/release/render-client
	UI_DIR = ../b/release/render-client
	PRE_TARGETDEPS += ../lib/librender.a
}

message(CONFIG $$CONFIG)

HEADERS		=	RCliMainWindowDialog.h \
				RCliMessageHolder.h \
				RCliRenderSocket.h \
				commands/RCommandWidget.h \
				commands/RBackgroundCommandWidget.h \
				commands/RFrameCommandWidget.h \
				commands/RFF2DCommandWidget.h \
				commands/RJSCommandWidget.h \
				commands/ROuterSpaceCommandWidget.h \
				commands/RConstantDensityPlaneCommandWidget.h \
				commands/RPerspectiveCommandWidget.h \
				commands/RCameraCommandWidget.h
				

SOURCES		=	main.cpp \
				RCliMainWindowDialog.cpp \
				RCliMessageHolder.cpp \
				RCliRenderSocket.cpp \
				commands/RCommandWidget.cpp \
				commands/RBackgroundCommandWidget.cpp \
				commands/RFrameCommandWidget.cpp \
				commands/RFF2DCommandWidget.cpp \
				commands/RJSCommandWidget.cpp \
				commands/ROuterSpaceCommandWidget.cpp \
				commands/RConstantDensityPlaneCommandWidget.cpp \
				commands/RPerspectiveCommandWidget.cpp \
				commands/RCameraCommandWidget.cpp
				
				
FORMS		=	RCliMainWindowForm.ui \
				commands/FrameCommandForm.ui \
				commands/FF2DCommandForm.ui \
				commands/JSCommandForm.ui \
				commands/OuterSpaceCommandForm.ui \
				commands/ConstantDensityPlaneForm.ui \
				commands/PerspectiveCommandForm.ui \
				commands/CameraCommandForm.ui

