# main compiler
cxx := g++

# define the directories
srcdir := src
incdir := include
blddir := build
bindir := bin
libdir := lib

# define common variables
srcext := cpp
sources := $(shell find $(srcdir) -maxdepth 1 -type f \( -iname "*.$(srcext)" ! -iname "*main-*.$(srcext)" \) )
objects := $(patsubst $(srcdir)/%, $(blddir)/%, $(sources: .$(srcext)=.o))
extra =
cflags := -g -fPIC $(extra) # -Wall
inc := $(shell find $(incdir) -maxdepth 1 -type d -exec echo -I {}  \;)

PYTHON_VERSION := 3.7
PYlibdir := lib/python$(PYTHON_VERSION)
PYinc := "-I/usr/include/python$(PYTHON_VERSION)m"
inc += $(PYinc)

# define specific binaries to create
# C libraries
LIBD := liboniondebug
SOURCE_LIBD := src/lib/onion-debug.$(srcext)
OBJECT_LIBD := $(patsubst $(srcdir)/%,$(blddir)/%,$(SOURCE_LIBD:.$(srcext)=.o))
TARGET_LIBD := $(libdir)/$(LIBD).so

LIB0 := libonioni2c
SOURCE_LIB0 := src/lib/onion-i2c.$(srcext)
OBJECT_LIB0 := $(patsubst $(srcdir)/%,$(blddir)/%,$(SOURCE_LIB0:.$(srcext)=.o))
TARGET_LIB0 := $(libdir)/$(LIB0).so
LIB_LIB0 := -L$(libdir) -loniondebug

LIB1 := libonionmcp23008
SOURCE_LIB1 := src/lib/onion-mcp23008-driver.$(srcext)
OBJECT_LIB1 := $(patsubst $(srcdir)/%,$(blddir)/%,$(SOURCE_LIB1:.$(srcext)=.o))
TARGET_LIB1 := $(libdir)/$(LIB1).so
LIB_LIB1 := -L$(libdir) -loniondebug -lonioni2c

LIB2 := libonionpwmexp
SOURCE_LIB2 := src/pwm-exp.$(srcext)
OBJECT_LIB2 := $(patsubst $(srcdir)/%,$(blddir)/%,$(SOURCE_LIB2:.$(srcext)=.o))
TARGET_LIB2 := $(libdir)/$(LIB2).so
LIB_LIB2 := -L$(libdir) -loniondebug -lonioni2c

LIB3 := libonionrelayexp
SOURCE_LIB3 := src/relay-exp.$(srcext)
OBJECT_LIB3 := $(patsubst $(srcdir)/%,$(blddir)/%,$(SOURCE_LIB3:.$(srcext)=.o))
TARGET_LIB3 := $(libdir)/$(LIB3).so
LIB_LIB3 := -L$(libdir) -loniondebug -lonioni2c -lonionmcp23008

LIB4 := libonionoledexp
SOURCE_LIB4 := src/oled-exp.$(srcext)
OBJECT_LIB4 := $(patsubst $(srcdir)/%,$(blddir)/%,$(SOURCE_LIB4:.$(srcext)=.o))
TARGET_LIB4 := $(libdir)/$(LIB4).so
LIB_LIB4 := -L$(libdir) -loniondebug -lonioni2c


# C applications
APP0 := pwm-exp
SOURCE_APP0 := $(srcdir)/main-$(APP0).$(srcext)
LIB_APP0 := -L$(libdir) -loniondebug -lonioni2c -lonionpwmexp
TARGET_APP0 := $(bindir)/$(APP0)

APP1 := relay-exp
SOURCE_APP1 := $(srcdir)/main-$(APP1).$(srcext)
LIB_APP1 := -L$(libdir) -loniondebug -lonioni2c -lonionmcp23008 -lonionrelayexp
TARGET_APP1 := $(bindir)/$(APP1)

APP2 := oled-exp
SOURCE_APP2 := $(srcdir)/main-$(APP2).$(srcext) $(srcdir)/$(APP2).$(srcext)
LIB_APP2 := -L$(libdir) -loniondebug -lonioni2c -lonionoledexp
TARGET_APP2 := $(bindir)/$(APP2)


# Python Modules
PYLIB00 := onionI2C
SOURCE_PYLIB00 := src/python/onion-i2c-module.$(srcext)
OBJECT_PYLIB00 := $(patsubst $(srcdir)/%,$(blddir)/%,$(SOURCE_PYLIB00:.$(srcext)=.o))
TARGET_PYLIB00 := $(PYlibdir)/$(PYLIB00).so
LIB_PYLIB00 := -L$(libdir) -loniondebug -lonioni2c -lpython$(PYTHON_VERSION)

PYLIB0 := pwmExp
SOURCE_PYLIB0 := src/python/pwm-exp-module.$(srcext)
OBJECT_PYLIB0 := $(patsubst $(srcdir)/%,$(blddir)/%,$(SOURCE_PYLIB0:.$(srcext)=.o))
TARGET_PYLIB0 := $(PYlibdir)/$(PYLIB0).so
LIB_PYLIB0 := -L$(libdir) -loniondebug -lonioni2c -lonionpwmexp -lpython$(PYTHON_VERSION)

PYLIB1 := relayExp
SOURCE_PYLIB1 := src/python/relay-exp-module.$(srcext)
OBJECT_PYLIB1 := $(patsubst $(srcdir)/%,$(blddir)/%,$(SOURCE_PYLIB1:.$(srcext)=.o))
TARGET_PYLIB1 := $(PYlibdir)/$(PYLIB1).so
LIB_PYLIB1 := -L$(libdir) -loniondebug -lonioni2c -lonionmcp23008 -lonionrelayexp -lpython$(PYTHON_VERSION)

PYLIB2 := oledExp
SOURCE_PYLIB2 := src/python/oled-exp-module.$(srcext)
OBJECT_PYLIB2 := $(patsubst $(srcdir)/%,$(blddir)/%,$(SOURCE_PYLIB2:.$(srcext)=.o))
TARGET_PYLIB2 := $(PYlibdir)/$(PYLIB2).so
LIB_PYLIB2 := -L$(libdir) -loniondebug -lonioni2c -lonionoledexp -lpython$(PYTHON_VERSION)



all: validate resp prepare $(TARGET_LIBD) $(TARGET_LIB0) $(TARGET_LIB1) $(TARGET_LIB2) $(TARGET_LIB3) $(TARGET_LIB4) $(TARGET_APP0) $(TARGET_APP1) $(TARGET_APP2)# $(TARGET_PYLIB00) $(TARGET_PYLIB0) $(TARGET_PYLIB1) $(TARGET_PYLIB2)


# libraries
$(TARGET_LIBD): $(OBJECT_LIBD)
	@echo " Compiling $@"
	@mkdir -p $(libdir)
	$(cxx) -shared -o $@  $^ $(LIB_LIBD)

$(TARGET_LIB0): $(OBJECT_LIB0)
	@echo " Compiling $@"
	@mkdir -p $(libdir)
	$(cxx) -shared -o $@  $^ $(LIB_LIB0)

$(TARGET_LIB1): $(OBJECT_LIB1)
	@echo " Compiling $@"
	@mkdir -p $(libdir)
	$(cxx) -shared -o $@  $^ $(LIB_LIB1)

$(TARGET_LIB2): $(OBJECT_LIB2)
	@echo " Compiling $@"
	@mkdir -p $(libdir)
	$(cxx) -shared -o $@  $^ $(LIB_LIB2)

$(TARGET_LIB3): $(OBJECT_LIB3)
	@echo " Compiling $@"
	@mkdir -p $(libdir)
	$(cxx) -shared -o $@  $^ $(LIB_LIB3)

$(TARGET_LIB4): $(OBJECT_LIB4)
	@echo " Compiling $@"
	@mkdir -p $(libdir)
	$(cxx) -shared -o $@  $^ $(LIB_LIB4)


# application binaries
$(TARGET_APP0): $(SOURCE_APP0)
	@echo " Compiling $(APP0)"
	@mkdir -p $(bindir)
	@echo " Linking..."
	$(cxx) $^ $(cflags) $(LDFLAGS) -o $(TARGET_APP0) $(LIB) $(LIB_APP0) -I$(incdir)

$(TARGET_APP1): $(SOURCE_APP1)
	@echo " Compiling $(APP1)"
	@mkdir -p $(bindir)
	@echo " Linking..."
	$(cxx) $^ $(cflags) $(LDFLAGS) -o $(TARGET_APP1) $(LIB) $(LIB_APP1) -I$(incdir)

$(TARGET_APP2): $(SOURCE_APP2)
	@echo " Compiling $(APP2)"
	@mkdir -p $(bindir)
	@echo " Linking..."
	$(cxx) $^ $(cflags) $(LDFLAGS) -o $(TARGET_APP2) $(LIB) $(LIB_APP2) -I$(incdir)


$(TARGET_PYLIB00): $(OBJECT_PYLIB00)
	@echo " Compiling $@"
	@mkdir -p $(PYlibdir)
	$(cxx) -shared -o $@  $^ $(LIB_PYLIB00)

$(TARGET_PYLIB0): $(OBJECT_PYLIB0)
	@echo " Compiling $@"
	@mkdir -p $(PYlibdir)
	$(cxx) -shared -o $@  $^ $(LIB_PYLIB0)

$(TARGET_PYLIB1): $(OBJECT_PYLIB1)
	@echo " Compiling $@"
	@mkdir -p $(PYlibdir)
	$(cxx) -shared -o $@  $^ $(LIB_PYLIB1)

$(TARGET_PYLIB2): $(OBJECT_PYLIB2)
	@echo " Compiling $@"
	@mkdir -p $(PYlibdir)
	$(cxx) -shared -o $@  $^ $(LIB_PYLIB2)


# generic: build any object file required
$(blddir)/%.o: $(srcdir)/%.$(srcext)
	@mkdir -p $(dir $@)
	@echo "compile $< $@"
	@if [ $< != "$(srcdir)/main-*.$(srcext)" ]; \
	then $(cxx) $(cflags) $(inc) -c -o $@ $<; \
	fi

clean:
	@echo " Cleaning..."; 
	$(RM) -r $(blddir) $(bindir) $(libdir)

bla:
	@echo "$(BLA)"
	@echo "TARGET_APP0: $(APP0) $(SOURCE_APP0) $(OBJECT_APP0) $(TARGET_APP0)"
	@echo "Sources: $(sources)"
	@echo "Objects: $(objects)"

resp:
	@echo "cxx: $(cxx)"
	@echo "cflags: $(cflags)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "LIB: $(LIB)"

validate:
ifeq ($(PYTHON_VERSION),)
$(error "PYTHON_VERSION variable is not set")
else
$(info "Using PYTHON_VERSION $(PYTHON_VERSION)")
endif

prepare:
	mkdir -p $(blddir)/lib $(bindir) $(libdir)
# Tests
tester:
	$(cxx) $(cflags) test/tester.$(srcext) $(inc) $(LIB) -o bin/tester

# Spikes
#ticket:
#  $(cxx) $(cflags) spikes/ticket.$(srcext) $(inc) $(LIB) -o bin/ticket

.PHONY: clean
