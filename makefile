cc = g++
EXECUTABLES = duke tc1 tc2 tc5 tc7 tc9 testREAD
CFLAGS = -Wall -lm -ggdb3 


all: ${EXECUTABLES}

duke: 1.cc 1t.o
	$(cc) $(CFLAGS) -m32 -o duke 1.cc 1t.o libinterrupt.a -ldl
tc1: tc1.cc 1t.o
	$(cc) $(CFLAGS) -m32 -o tc1 tc1.cc 1t.o libinterrupt.a -ldl
tc2: tc2.cc 1t.o
	$(cc) $(CFLAGS) -m32 -o tc2 tc2.cc 1t.o libinterrupt.a -ldl
tc5: tc5.cc 1t.o
	$(cc) $(CFLAGS) -m32 -o tc5 tc5.cc 1t.o libinterrupt.a -ldl
tc7: tc7.cc 1t.o
	$(cc) $(CFLAGS) -m32 -o tc7 tc7.cc 1t.o libinterrupt.a -ldl
tc9: tc9.cc 1t.o
	$(cc) $(CFLAGS) -m32 -o tc9 tc9.cc 1t.o libinterrupt.a -ldl


testREAD: testREAD.cc 1t.o
	$(cc) $(CFLAGS) -m32 -o testREAD testREAD.cc 1t.o libinterrupt.a -ldl


1t.o:1t.cc
	$(cc) $(CFLAGS) -m32 -c 1t.cc libinterrupt.a -ldl
clean:
	rm -f ${EXECUTABLES} 1t.o
