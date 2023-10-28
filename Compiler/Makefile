B=build
CPP=g++
CPP_FLAGS=-Wall -Werror -g -std=c++17
CC=gcc
CC_FLAGS=-Wall -Werror -g -std=c99

CPP_FILES=${wildcard *.cpp}
CPP_O_FILES=${addprefix $B/,${subst .cpp,.o,${CPP_FILES}}}

C_FILES=${wildcard *.c}
C_O_FILES=${addprefix $B/,${subst .c,.o,${C_FILES}}}

LINK=${firstword ${patsubst %.cpp,${CPP},${CPP_FILES} ${patsubst %.c,${CC},${C_FILES}}}}
LINK_FLAGS=

all : $B/main

$B/main: ${CPP_O_FILES} ${C_O_FILES}
	@mkdir -p build
	${LINK} -o $@ ${LINK_FLAGS} ${CPP_O_FILES} ${C_O_FILES}

${CPP_O_FILES} : $B/%.o: %.cpp Makefile
	@mkdir -p build
	${CPP} -MMD -MF $B/$*.d -c -o $@ ${CPP_FLAGS} $*.cpp

${C_O_FILES} : $B/%.o: %.c Makefile
	@mkdir -p build
	${CC} -MMD -MF $B/$*.d -c -o $@ ${CC_FLAGS} $*.c

-include $B/*.d

clean:
	rm -rf build
