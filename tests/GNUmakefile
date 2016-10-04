# vim: set tabstop=8 shiftwidth=8 noexpandtab:

CSRCS_DIR := ../src
OBJS_DIR := build

$(shell mkdir -p ${OBJS_DIR} > /dev/null)

VPATH = ${CSRCS_DIR}

CPPFLAGS += -I${CPPUTEST_INC} -g -Wall
CXXFLAGS += -include ${CPPUTEST_INC}/CppUTest/MemoryLeakDetectorNewMacros.h
CXXFLAGS += -std=c++11 -stdlib=libc++
CFLAGS += -include ${CPPUTEST_INC}/CppUTest/MemoryLeakDetectorMallocMacros.h
CFLAGS += -I${CSRCS_DIR}
LDFLAGS += -L${CPPUTEST_LIBDIR}
LDLIBS += -lCppUTest -lCppUTestExt

PROG := testsRunner

CXXSRCS := $(notdir $(wildcard *.cpp))
CSRCS := $(notdir $(wildcard ${CSRCS_DIR}/*.c))
OBJS := $(addsuffix .o,$(basename ${CSRCS} ${CXXSRCS}))
OBJS := $(addprefix ${OBJS_DIR}/,${OBJS})
PROG := $(addprefix ${OBJS_DIR}/,${PROG})

#all	: view ${PROG}
all	: ${PROG}

${OBJS_DIR}/%.o	: %.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} ${DEPFLAGS} -c -o $@ $<

${OBJS_DIR}/%.o	: %.c
	${CC} ${CPPFLAGS} ${CFLAGS} ${DEPFLAGS} -c -o $@ $<

${PROG} : ${OBJS}
	${CXX} ${CPPFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

view    :
	@echo "CWD    : `pwd`"
	@echo "CXXSRCS: ${CXXSRCS}"
	@echo "CSRCS  : ${CSRCS}"
	@echo "PROG   : ${PROG}"
	@echo "OBJS   : ${OBJS}"

clean   :
	rm -rf *.core ${PROG} ${OBJS}

