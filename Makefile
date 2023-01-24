# install commands
INSTALL := install
INSTALL_PROGRAM := $(INSTALL)
INSTALL_DATA := ${INSTALL} -m 644

# bin and lib dirs
LIBDIR := lib
BINDIR := bin
MODDIR := mod

# Additional scripts that are packaged 
SCRIPTDIR := scripts

# Directories (relative to project root) of various sub-projects

RCLIDIR := render-client
RJCLIDIR := render-js-client
RCMDDIR := render-command-line
RDOTDIR := render-dots
RENDIODIR := rendio
VPIXXDIR := vpixx
RENDIOTESTDIR := rendio-test

# librender dirs
LIBSRC := librender/src
OSGSRC := $(LIBSRC)/osg
UTILSRC := $(LIBSRC)/util
R3CSRC := $(LIBSRC)/r3c
JSONQSRC := $(LIBSRC)/jsonq
BBUFFSRC := $(LIBSRC)/bbuff
SPECSRC := $(LIBSRC)/spec
CORESRC := $(LIBSRC)/core
FXMHSRC := $(LIBSRC)/fxmh
MSGQSRC := $(LIBSRC)/msgq
ANIMSRC := $(LIBSRC)/anim
TRISRC  := $(LIBSRC)/tri
INCLUDES := -I$(LIBSRC) -I$(OSGSRC) -I$(UTILSRC) -I$(R3CSRC) -I$(JSONQSRC) -I$(BBUFFSRC) -I$(SPECSRC) -I$(CORESRC) -I$(FXMHSRC) -I$(MSGQSRC) -I$(ANIMSRC) -I$(TRISRC) -I$(RENDIODIR)

# source and headers for library
LIBSRCS := $(shell find $(LIBSRC) -type f -name "*.cpp")
LIBSRCS := $(filter-out librender/src/GpibMessageQueue.cpp,$(LIBSRCS))
LIBHDRS := $(shell find $(LIBSRC) -type f -name "*.h")
LIBHDRS := $(filter-out librender/src/GpibMessageQueue.h,$(LIBHDRS))
LIBINLS := $(shell find $(LIBSRC) -type f -name "*.inl")


# VERSION defaults to RELEASE. 
# set to DEBUG on command line with make VERSION=DEBUG
VERSION = RELEASE

CWARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wconversion -Wstrict-prototypes

# in CXX add no-deprecated because OSG src has a LOT of these. 
CXXWARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-declarations \
            -Wredundant-decls -Winline -Wno-long-long \
            -Wuninitialized -Wconversion -Wno-deprecated


ifeq "$(VERSION)" "RELEASE"
	CFLAGS := -O2 -std=c99 $(CWARNINGS) $(INCLUDES)
	CXXFLAGS := -O2 -std=c++11 $(CXXWARNINGS) $(INCLUDES)
	BUILDTYPE := release
	LINKLIB := render
	RENDERBIN := render
	RCLIBIN := render-client
	RCLIBUILDARG := release
	RJCLIBIN := render-js-client
	RCMDBIN := render-command-line
	RDOTBIN := render-dots
	VPIXXBIN := vpixx
	RENDIOKO := rendio.ko
	RENDIOTESTBIN := rendio-test
endif
ifeq "$(VERSION)" "DEBUG"
	CFLAGS := -g -std=c99 $(CWARNINGS) $(INCLUDES)
	CXXFLAGS := -g -std=c++11 $(CXXWARNINGS) $(INCLUDES)
	BUILDTYPE := debug
	LINKLIB := renderd
	RENDERBIN := renderd
	RCLIBIN := render-clientd
	RCLIBUILDARG := debug
	RJCLIBIN := render-js-clientd
	RCMDBIN := render-command-lined
	RDOTBIN := render-dotsd
	VPIXXBIN := vpixxd
	RENDIOKO := rendio.ko
	RENDIOTESTBIN := rendio-testd
endif

# targets to be built. These should point to the actual executable/lib/module to be built
LIBTARGET := $(LIBDIR)/lib$(LINKLIB).a
RENDERTARGET := $(BINDIR)/$(RENDERBIN)
RCLITARGET := $(BINDIR)/$(RCLIBIN)
RJCLITARGET := $(BINDIR)/$(RJCLIBIN)
RCMDTARGET := $(BINDIR)/$(RCMDBIN)
RDOTTARGET := $(BINDIR)/$(RDOTBIN)
VPIXXTARGET := $(BINDIR)/$(VPIXXBIN)
RENDIOTARGET := $(MODDIR)/$(RENDIOKO)
RENDIOTESTTARGET := $(BINDIR)/$(RENDIOTESTBIN)

#$(info $(RENDIOTARGET))
#$(info "Build type is " $(BUILDTYPE))


# build directories
ROOTBUILDDIR := b
BUILDDIR := $(ROOTBUILDDIR)/$(BUILDTYPE)


# object files for library
###LIBOBJS := $(patsubst %.cpp, %.o, $(LIBSRCS))

LIBOBJS := $(patsubst %.cpp, $(BUILDDIR)/%.o, $(LIBSRCS))
LIBDEPS := $(patsubst %.o, %.d, $(LIBOBJS))
#$(info $(LIBOBJS))
#$(info $(LIBDEPS))

# library dependency files.
###LIBDEPS := $(patsubst %.cpp,%.d,$(LIBSRCS))
-include $(LIBDEPS)


# render dirs
RENDERSRC := render/src

# sources, objects and deps for render
RENDERSRCS := $(shell find $(RENDERSRC) -type f -name "*.cpp")
#$(info "RENDERSRCS " $(RENDERSRCS))
RENDEROBJS := $(patsubst %.cpp, $(BUILDDIR)/%.o, $(RENDERSRCS))
RENDERDEPS := $(patsubst %.o, %.d, $(RENDEROBJS))
#$(info $(RENDEROBJS))
#$(info $(RENDERDEPS))

-include $(RENDERDEPS)

# libs needed to link render
# Note - uses release/debug version of librender, but always uses opt version of osg libs.
GLFW3LIBS := -lglfw3 -lGL -lm -lXrandr -lXi -lX11 -lXxf86vm -lXinerama -lXcursor -lpthread
RENDERLIBS := -L$(LIBDIR) -l$(LINKLIB) -lboost_log -lboost_thread -lboost_system -lboost_program_options $(GLFW3LIBS) `osg-config --prefix=/usr/lib --opt --libs Base System X`  
RCMDLIBS := -lpopt -lncurses -L$(LIBDIR) -l$(LINKLIB)
RDOTLIBS := -lglfw3 -lGL -lm -lXrandr -lXi -lX11 -lXxf86vm -lXinerama -lXcursor -lpthread
RJCLILIBS := -L$(LIBDIR) -l$(LINKLIB) -lboost_program_options -lboost_system -lpthread 

# djs -L/usr/local/lib -lparapin 
# must re-integrate parapin stuff


# object files for render-command-line
RCMDSRC := render-command-line
RCMDSRCS := $(RCMDSRC)/main.cpp
RCMDOBJS := $(patsubst %.cpp, $(BUILDDIR)/%.o, $(RCMDSRCS))
$(info $(RCMDSRCS))
$(info $(RCMDOBJS))

# object files for render-dots
RDOTSRC := render-dots
RDOTSRCS := $(RDOTSRC)/render-dots.cpp
RDOTOBJS := $(patsubst %.cpp, $(BUILDDIR)/%.o, $(RDOTSRCS))
$(info $(RDOTSRCS))
$(info $(RDOTOBJS))

# object files for render-js-client
RJCLISRC := render-js-client
RJCLISRCS := $(RJCLISRC)/render-js-client.cpp
RJCLIOBJS := $(patsubst %.cpp, $(BUILDDIR)/%.o, $(RJCLISRCS))
$(info $(RJCLISRCS))
$(info $(RJCLIOBJS))

# object files et al for vpixx
VPIXXSRCFILES	= 	$(VPIXXDIR)/vpixx.c \
					$(VPIXXDIR)/libdpx/src/libdpx.c \
					$(VPIXXDIR)/libusb/linux.c \
					$(VPIXXDIR)/libusb/descriptors.c \
					$(VPIXXDIR)/libusb/error.c \
					$(VPIXXDIR)/libusb/usb.c
VPIXXHDRFILES	= 	$(VPIXXDIR)/libdpx/src/libdpx.h \
		  			$(VPIXXDIR)/libdpx/src/libdpx_i.h \
		  			$(VPIXXDIR)/libusb/config.h \
		  			$(VPIXXDIR)/libusb/usb.h \
		  			$(VPIXXDIR)/libusb/usbi.h \
		  			$(VPIXXDIR)/libusb/error.h
VPIXXINC		=	-I$(VPIXXDIR)/libdpx/src -I$(VPIXXDIR)/libusb
VPIXXLIB		=	-lm -lpopt

# object files et al for rendio-test
RENDIOTESTSRCFILES	= $(RENDIOTESTDIR)/rendio-test.cpp
RENDIOTESTHDRFILES	=
RENDIOTESTINC		= 
RENDIOTESTLIB		= -lcomedi


# compilation rules

# build rule for lib files
$(BUILDDIR)/$(LIBSRC)/%.o: $(LIBSRC)/%.cpp
	@echo "===== Building $@"
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# build rule for render srcs
$(BUILDDIR)/$(RENDERSRC)/%.o: $(RENDERSRC)/%.cpp
	@echo "===== Building $@"
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# build rule for render-command-line srcs
$(BUILDDIR)/$(RCMDSRC)/%.o: $(RCMDSRC)/%.cpp
	@echo "===== Building $@ for render-command-line"
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# build rule for render-dots srcs
$(BUILDDIR)/$(RDOTSRC)/%.o: $(RDOTSRC)/%.cpp
	@echo "===== Building $@ for render-dots"
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# build rule for render-js-client srcs
$(BUILDDIR)/$(RJCLISRC)/%.o: $(RJCLISRC)/%.cpp
	@echo "===== Building $@ for render-js-client"
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@


all: directories librender render render-client render-js-client render-command-line render-dots vpixx rendio rendio-test

# hack render-client needs to be included above

# rule to create output directories. 
directories:
	@mkdir -p $(BINDIR)
	@mkdir -p $(LIBDIR)
	@mkdir -p $(MODDIR)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(BUILDDIR)/$(LIBSRC)
	@mkdir -p $(BUILDDIR)/$(OSGSRC)
	@mkdir -p $(BUILDDIR)/$(UTILSRC)
	@mkdir -p $(BUILDDIR)/$(SPECSRC)
	@mkdir -p $(BUILDDIR)/$(R3CSRC)
	@mkdir -p $(BUILDDIR)/$(JSONQSRC)
	@mkdir -p $(BUILDDIR)/$(BBUFFSRC)
	@mkdir -p $(BUILDDIR)/$(CORESRC)
	@mkdir -p $(BUILDDIR)/$(FXMHSRC)
	@mkdir -p $(BUILDDIR)/$(MSGQSRC)
	@mkdir -p $(BUILDDIR)/$(ANIMSRC)
	@mkdir -p $(BUILDDIR)/$(TRISRC)	
	@mkdir -p $(BUILDDIR)/$(RENDERSRC)
	@mkdir -p $(BUILDDIR)/render-client
	@mkdir -p $(BUILDDIR)/render-js-client
	@mkdir -p $(BUILDDIR)/render-command-line
	@mkdir -p $(BUILDDIR)/render-dots
	@mkdir -p $(BUILDDIR)/render-test
	
 
librender: $(LIBTARGET)

$(LIBTARGET): $(LIBOBJS)
	$(AR) r $(LIBTARGET) $?

render: $(RENDERTARGET)

$(RENDERTARGET): $(RENDEROBJS) $(LIBTARGET)
	@echo "===== Building $@ for $(BUILDTYPE)"
	$(CXX) $(CPPFLAGS) -o $@ $(RENDEROBJS) $(RENDERLIBS)

render-client: $(RCLITARGET)

$(RCLITARGET): librender
	@echo "===== Building $@ for $(BUILDTYPE)"
	cd $(RCLIDIR) && $(MAKE) $(RCLIBUILDARG)
		

render-command-line: $(RCMDTARGET)

$(RCMDTARGET): $(RCMDOBJS) $(LIBTARGET)
	@echo "===== Building $@ for $(BUILDTYPE)"
	$(CXX) $(CPPFLAGS) -o $@ $< $(RCMDLIBS)

render-dots: $(RDOTTARGET)

$(RDOTTARGET): $(RDOTOBJS)
	@echo "===== Building $@ for $(BUILDTYPE)"
	$(CXX) $(CPPFLAGS) -o $@ $< $(RDOTLIBS)

render-js-client: $(RJCLITARGET)

$(RJCLITARGET): $(RJCLIOBJS) $(LIBTARGET)
	@echo "===== Building $@ for $(BUILDTYPE)"
	$(CXX) $(CPPFLAGS) -o $@ $< $(RJCLILIBS)

vpixx: $(VPIXXTARGET)

$(VPIXXTARGET): $(VPIXXSRCFILES) $(VPIXXHDRFILES)
	@echo "===== Building $@ for $(BUILDTYPE)"
	$(CC) $(CPPFLAGS) $(VPIXXINC) -o $@ $(VPIXXSRCFILES) $(VPIXXLIB)
	
rendio: $(RENDIOTARGET)

$(RENDIOTARGET):
	@echo "===== Building $@ for $(BUILDTYPE)"
	cd $(RENDIODIR) && $(MAKE)

rendio-test: $(RENDIOTESTTARGET)

$(RENDIOTESTTARGET): $(RENDIOTESTSRCFILES) $(RENDIOTESTHDRFILES)
	@echo "===== Building $@ for $(BUILDTYPE)"
	$(CXX) $(CPPFLAGS) $(RENDIOINV) -o $@ $(RENDIOTESTSRCFILES) $(RENDIOTESTLIB)

clean:
	$(RM) $(LIBOBJS) $(LIBDEPS) $(LIBTARGET) $(RENDEROBJS) $(RENDERDEPS) $(RENDERTARGET) $(RCLIOBJS) $(RDOTOBJS) $(VPIXXTARGET)

distclean: clean debclean
	$(RM) -rf $(BINDIR)
	$(RM) -rf $(LIBDIR)
	$(RM) -rf $(BUILDDIR)
	
debclean:
	$(RM) -f cnsrender*.deb

# djs	cd $(RCLIDIR) && $(MAKE) clean


install: all
	$(INSTALL) -d $(DESTDIR)/usr/bin
	$(INSTALL_PROGRAM) $(RENDERTARGET) $(DESTDIR)/usr/bin
#	$(INSTALL_PROGRAM) $(RCLITARGET) $(DESTDIR)/usr/bin
	$(INSTALL_PROGRAM) $(RCMDTARGET) $(DESTDIR)/usr/bin
	$(INSTALL_PROGRAM) $(VPIXXTARGET) $(DESTDIR)/usr/bin
	$(INSTALL_PROGRAM) $(RDOTTARGET) $(DESTDIR)/usr/bin
	$(INSTALL_PROGRAM) $(RENDIOTESTTARGET) $(DESTDIR)/usr/bin	
	$(INSTALL_PROGRAM) $(SCRIPTDIR)/render-session $(DESTDIR)/usr/bin
	$(INSTALL_PROGRAM) $(SCRIPTDIR)/run-render $(DESTDIR)/usr/bin
	$(INSTALL) -d $(DESTDIR)/usr/share/xsessions
	$(INSTALL_PROGRAM) -m 0644 $(SCRIPTDIR)/render.desktop $(DESTDIR)/usr/share/xsessions
	$(INSTALL) -d $(DESTDIR)/etc/security/limits.d
	$(INSTALL_PROGRAM) -m 0640 $(SCRIPTDIR)/render-limits.conf $(DESTDIR)/etc/security/limits.d
	$(INSTALL) -d $(DESTDIR)/etc/sudoers.d
	$(INSTALL_PROGRAM) -m 0440 $(SCRIPTDIR)/render-sudoers $(DESTDIR)/etc/sudoers.d
	$(INSTALL) -d $(DESTDIR)/etc/rsyslog.d
	$(INSTALL_PROGRAM) -m 0440 $(SCRIPTDIR)/99-render-rsyslog.conf $(DESTDIR)/etc/rsyslog.d
	$(INSTALL) -d $(DESTDIR)/etc/init.d
	$(INSTALL_PROGRAM) -m 0755 $(SCRIPTDIR)/vpixx-init.d $(DESTDIR)/etc/init.d/vpixx
	$(INSTALL) -d $(DESTDIR)/etc/default
	$(INSTALL_PROGRAM) -m 0644 $(SCRIPTDIR)/vpixx-default $(DESTDIR)/etc/default/vpixx
	$(INSTALL) -d $(DESTDIR)/lib/udev/rules.d
	$(INSTALL_PROGRAM) -m 0644 $(SCRIPTDIR)/99-rendio.rules $(DESTDIR)/lib/udev/rules.d
	
.PHONY: all clean directories dist check librender render render-client deb debclean render-command-line render-dots
.DEFAULT_GOAL := all