# vim: set tabstop=8 shiftwidth=8 noexpandtab:

TESTS_CMN_DIR := ../common
CSRCS_DIR := ../../src
OBJS_DIR := build

$(shell mkdir -p ${OBJS_DIR} > /dev/null)

VPATH = ${CSRCS_DIR}:${TESTS_CMN_DIR}

CPPFLAGS += -I${CPPUTEST_INC} -I${CSRCS_DIR}
CPPFLAGS += -g -Wall
CPPFLAGS += -DTESTBUILD
CXXFLAGS += -include ${CPPUTEST_INC}/CppUTest/MemoryLeakDetectorNewMacros.h
CXXFLAGS += -std=c++11 -stdlib=libc++
CXXFLAGS += -I${TESTS_CMN_DIR}
CFLAGS += -include ${CPPUTEST_INC}/CppUTest/MemoryLeakDetectorMallocMacros.h
LDFLAGS += -L${CPPUTEST_LIBDIR}
LDLIBS += -lCppUTest -lCppUTestExt

TEST_TARGET := LCDIntf.c

PROG := testsRunner

CXXSRCS := $(notdir $(wildcard *.cpp ${TESTS_CMN_DIR}/*.cpp))
CSRCS := $(notdir $(wildcard ${CSRCS_DIR}/${TEST_TARGET} ${TESTS_CMN_DIR}/*.c))
OBJS := $(addsuffix .o,$(basename ${CSRCS} ${CXXSRCS}))
OBJS := $(addprefix ${OBJS_DIR}/,${OBJS})
PROG := $(addprefix ${OBJS_DIR}/,${PROG})

#all	: view ${PROG}
all	: ${PROG}

${OBJS_DIR}/%.o	: %.cpp
	${CXX} ${CXXFLAGS} ${CPPFLAGS} ${DEPFLAGS} -c -o $@ $<

${OBJS_DIR}/%.o	: %.c
	${CC} ${CFLAGS} ${CPPFLAGS} ${DEPFLAGS} -c -o $@ $<

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

