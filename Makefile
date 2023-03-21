CFLAGS = -Werror -Wall -O0 -g -std=c11

LIB_C_FILES=${wildcard src/*.c}
LIB_S_FILES=${wildcard src/*.s}
LIB_OC_FILES=${subst .c,.o,${LIB_C_FILES}}
LIB_OS_FILES=${subst .s,.o,${LIB_S_FILES}}
LIB_O_FILES=${LIB_OS_FILES} ${LIB_OC_FILES}
C_FILES=${wildcard *.c}
O_FILES=${subst .c,.o,${C_FILES}}

all : ${LIB_O_FILES}

${LIB_OC_FILES} : %.o : Makefile %.c
	gcc -MMD ${CFLAGS} -c -o $@ $*.c

${LIB_OS_FILES} : %.o : Makefile %.s
	gcc -MMD ${CFLAGS} -c -o $@ $*.s

${O_FILES} : %.o : Makefile %.c
	gcc -MMD ${CFLAGS} -c -o $@ $*.c

clean:
	-rm -rf ${PROG} *.out *.diff *.result *.d *.o src/*.d src/*.o *.time *.err *.run

-include *.d
-include src/*.d