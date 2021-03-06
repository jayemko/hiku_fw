CXX := $(HOME)/arm-none-eabi/bin/arm-none-eabi-g++
AR := $(HOME)/arm-none-eabi/bin/arm-none-eabi-ar

OPENCV_ROOT := ../opencv-atmel/install
OPENCV_HEADER_DIRS := $(shell find $(OPENCV_ROOT)/include -type d)
HEADER_DIRS := $(shell find include -type d)
CXXFLAGS := -Wall -Og -g -fPIC $(foreach dir, $(HEADER_DIRS), -I$(dir)) $(foreach dir, $(OPENCV_HEADER_DIRS), -I$(dir))
LDFLAGS	:= -Wall -Og -g -fPIC -Wl,--no-undefined

HEADERS := $(wildcard $(addsuffix /*, $(HEADER_DIRS)))
HFILES := $(filter %.h, $(HEADERS))
HPPFILES := $(filter %.hpp, $(HEADERS))

SOURCES := $(filter %.cpp, $(wildcard $(addsuffix /*, $(shell find source -type d))))

OBJECTS = $(SOURCES:%.cpp=%.o)

static: libopenqr.a

libopenqr.a: $(OBJECTS)
	$(AR) rcvs $@ $(OBJECTS)

%.o: %.cpp $(HFILES) $(HPPFILES)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%: %.o static
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LDFLAGS) $(LDLIBS)

.PHONY: clean install uninstall

clean:
	rm -f *.a
	$(shell find . -name "*.o" -exec rm '{}' ';')

uninstall:
	rm -rf install

install: uninstall
	mkdir -p install/include install/lib
	cp $(HFILES) $(HPPFILES) install/include/
	mv *.a install/lib/
	$(MAKE) clean
