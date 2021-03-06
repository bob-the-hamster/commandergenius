
$(shell mkdir -p AI Scripting debug/AI debug/Scripting \
	out/vcmi/lib out/vcmi/lib/Filesystem out/vcmi/lib/Mapping \
	out/vcmi/lib/RMG out/vcmi/server out/vcmi/AI/StupidAI \
	out/vcmi/AI/BattleAI out/vcmi/AI/FuzzyLite out/vcmi/AI/VCAI \
	out/vcmi/AI/EmptyAI out/vcmi/Scripting/ERM out/vcmi/client \
	out/vcmi/client/BattleInterface out/vcmi/client/UIFramework)

LOCAL_PATH=$(shell dirname $0)
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

GCC_PREFIX=$(if $(shell which ndk-build | grep 'android-ndk-r8'),arm-linux-androideabi,arm-eabi)

SOURCES_LIB=$(wildcard vcmi/lib/*.cpp vcmi/CConsoleHandler.cpp vcmi/CThreadHelper.cpp)
OBJS_LIB=$(patsubst %.cpp, out/%.o, $(SOURCES_LIB))

SOURCES_LIB_FILESYSTEM:=$(wildcard vcmi/lib/Filesystem/*.cpp)
OBJS_LIB_FILESYSTEM:=$(patsubst %.cpp, out/%.o, $(SOURCES_LIB_FILESYSTEM))

SOURCES_LIB_MAPPING:=$(wildcard vcmi/lib/Mapping/*.cpp)
OBJS_LIB_MAPPING:=$(patsubst %.cpp, out/%.o, $(SOURCES_LIB_MAPPING))

SOURCES_LIB_RMG:=$(wildcard vcmi/lib/RMG/*.cpp)
OBJS_LIB_RMG:=$(patsubst %.cpp, out/%.o, $(SOURCES_LIB_RMG))

SOURCES_SERVER:=$(wildcard vcmi/server/*.cpp)
OBJS_SERVER:=$(patsubst %.cpp, out/%.o, $(SOURCES_SERVER))

SOURCES_CLIENT:=$(wildcard vcmi/client/*.cpp vcmi/client/BattleInterface/*.cpp vcmi/client/UIFramework/*.cpp vcmi/*.cpp)
OBJS_CLIENT:=$(patsubst %.cpp, out/%.o, $(SOURCES_CLIENT))

SOURCES_BATTLEAI:=$(wildcard vcmi/AI/BattleAI/*.cpp)
OBJS_BATTLEAI:=$(patsubst %.cpp, out/%.o, $(SOURCES_BATTLEAI))

SOURCES_EMPTYAI:=$(wildcard vcmi/AI/EmptyAI/*.cpp)
OBJS_EMPTYAI:=$(patsubst %.cpp, out/%.o, $(SOURCES_EMPTYAI))

SOURCES_FUZZYLITE:=$(wildcard vcmi/AI/FuzzyLite/*.cpp)
OBJS_FUZZYLITE:=$(patsubst %.cpp, out/%.o, $(SOURCES_FUZZYLITE))

SOURCES_STUPIDAI:=$(wildcard vcmi/AI/StupidAI/*.cpp)
OBJS_STUPIDAI:=$(patsubst %.cpp, out/%.o, $(SOURCES_STUPIDAI))

SOURCES_VCAI:=$(wildcard vcmi/AI/VCAI/*.cpp)
OBJS_VCAI:=$(patsubst %.cpp, out/%.o, $(SOURCES_VCAI))

SOURCES_ERM:=$(wildcard vcmi/Scripting/ERM/*.cpp)
OBJS_ERM:=$(patsubst %.cpp, out/%.o, $(SOURCES_ERM))

all: AndroidData/binaries.zip libapplication.so

AndroidData/binaries.zip: vcmiserver libvcmi.so Scripting/libvcmiERM.so AI/libBattleAI.so AI/libEmptyAI.so AI/libStupidAI.so AI/libVCAI.so
	rm -f $@
	zip -r $@ $^

# With default -O3 optimization each G++ process eats 2Gb RAM, so tone optimization down to -O2, and limit make to two jobs
$(OBJS_SERVER) $(OBJS_LIB) $(OBJS_LIB_FILESYSTEM) $(OBJS_LIB_MAPPING) $(OBJS_LIB_RMG) $(OBJS_BATTLEAI) $(OBJS_EMPTYAI) $(OBJS_FUZZYLITE) $(OBJS_VCAI) $(OBJS_CLIENT) $(OBJS_ERM): out/%.o: %.cpp
	../setEnvironment.sh sh -c \
	"$(GCC_PREFIX)-g++ \
	-c \$$CXXFLAGS -O2 -Ivcmi -std=c++11 -Ivcmi/lib \
	-DM_DATA_DIR=\\\".\\\" \
	-DM_BIN_DIR=\\\"/data/data/eu.vcmi/files\\\" \
	-DM_LIB_DIR=\\\"/data/data/eu.vcmi/files\\\" \
	-Wstrict-aliasing -Wcast-align -Wpointer-arith -Waddress \
	$< -o $@"
#	-Werror=strict-aliasing -Werror=cast-align -Werror=pointer-arith -Werror=address

vcmiserver: $(OBJS_SERVER) -lvcmi
	env BUILD_EXECUTABLE=1 NO_SHARED_LIBS=1 ../setEnvironment.sh sh -c \
	"$(GCC_PREFIX)-g++ \
	$^ -o $@ \
	\$$LDFLAGS -L. -L../../boost/lib/arm-linux-androideabi-4.6 \
	-lboost_filesystem -lboost_iostreams -lboost_system -lboost_thread -lgnustl_static" && \
	cp $@ debug/$@ && \
	$(GCC_PREFIX)-strip $@

LINK_LIB= \
	env BUILD_EXECUTABLE=1 NO_SHARED_LIBS=1 ../setEnvironment.sh sh -c \
	"$(GCC_PREFIX)-g++ \
	$^ -o $@ \
	-shared \$$LDFLAGS -L. -L../../boost/lib/arm-linux-androideabi-4.6 \
	-lboost_filesystem -lboost_iostreams -lboost_system -lboost_thread -lgnustl_static" && \
	cp $@ debug/$@ && \
	$(GCC_PREFIX)-strip $@

.PHONY: -lvcmi
-lvcmi: libvcmi.so

AI/libBattleAI.so: $(OBJS_BATTLEAI) -lvcmi
	$(LINK_LIB)

AI/libEmptyAI.so: $(OBJS_EMPTYAI) -lvcmi
	$(LINK_LIB)

AI/libStupidAI.so: $(OBJS_STUPID) -lvcmi
	$(LINK_LIB)

AI/libVCAI.so: $(OBJS_VCAI) $(OBJS_FUZZYLITE) -lvcmi
	$(LINK_LIB)

Scripting/libvcmiERM.so: $(OBJS_ERM) -lvcmi
	$(LINK_LIB)

libvcmi.so: $(OBJS_LIB) $(OBJS_LIB_FILESYSTEM) $(OBJS_LIB_MAPPING) $(OBJS_LIB_RMG)
	$(LINK_LIB)

libapplication.so: $(OBJS_CLIENT) libvcmi.so
	env ../setEnvironment.sh sh -c \
	"$(GCC_PREFIX)-g++ $^ -o $@ \
	\$$LDFLAGS -L. -L../../boost/lib/arm-linux-androideabi-4.6 -lboost_program_options -lboost_filesystem -lboost_iostreams -lboost_system -lboost_thread -lgnustl_static"

clean:
	rm -rf out debug *.so vcmiserver* AndroidData/vcmiserver* libapplication.so AI Scripting

.PHONY: clean

