B=build
CPP=g++
CPP_FLAGS=-Wall -Werror -g -std=c++17
CC=gcc
CC_FLAGS=-Wall -Werror -g -std=c99

CPP_FILES=${wildcard *.cpp}
CPP_O_FILES=${addprefix $B/,${subst .cpp,.o,${CPP_FILES}}}

C_FILES=${wildcard *.c}
C_O_FILES=${addprefix $B/,${subst .c,.o,${C_FILES}}}

TESTS_DIR=sample
TEST_FILES=${wildcard *.c}
TEST_O_FILES=${addprefix ${TESTS_DIR}/$B/,${subst .c,.o,${TEST_FILES}}}

LINK=${firstword ${patsubst %.cpp,${CPP},${CPP_FILES} ${patsubst %.c,${CC},${C_FILES}}}}
LINK_FLAGS=

all : $B/main

# test : all ${TESTS_DIR}/$B/main
test : all
	@mkdir -p ${TESTS_DIR}/$B
	./build/main sample/sample.canyon sample/sample.c 2> /dev/null
	gcc sample/sample.c -o sample/build/main
	./sample/build/main

$B/main: ${CPP_O_FILES} ${C_O_FILES}
	@mkdir -p $B
	${LINK} -o $@ ${LINK_FLAGS} ${CPP_O_FILES} ${C_O_FILES}

${CPP_O_FILES} : $B/%.o: %.cpp Makefile
	@mkdir -p build
	${CPP} -MMD -MF $B/$*.d -c -o $@ ${CPP_FLAGS} $*.cpp

${C_O_FILES} : $B/%.o: %.c Makefile
	@mkdir -p build
	${CC} -MMD -MF $B/$*.d -c -o $@ ${CC_FLAGS} $*.c

# ${TESTS_DIR}/$B/main: ${TEST_O_FILES}
# 	@mkdir -p ${TESTS_DIR}/$B
# 	${LINK} -o $@ ${LINK_FLAGS} ${TEST_O_FILES}

# ${TEST_O_FILES} : ${TESTS_DIR}/$B/%.o: ${TESTS_DIR}/%.c Makefile
# 	@mkdir -p ${TESTS_DIR}/$B
# 	${CC} -MMD -MF ${TESTS_DIR}/$B/$*.d -c -o $@ ${CC_FLAGS} $*.c

-include $B/*.d

clean:
	rm -rf $B
	rm -rf ${TESTS_DIR}/$B
	rm -f ${TESTS_DIR}/*.c
