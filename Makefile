# Important stuff
CFLAGS = 
LIBS = -luuid -ldxguid -ldinput -ldinput8

# (maybe) distro-specific stuff
MINGW32 = i686-w64-mingw32
MINGW64 = i686-w64-mingw32
BIN = /usr/bin
USR = /usr
LIB = lib/			# used in as ${USR}/${MINGWxy}/${LIB}
INCLUDE = include/	# used in as ${USR}/${MINGWxy}/${INCLUDE}

# Computed
GCC32 = ${BIN}/${MINGW32}-gcc
INCLUDE_DIR32 = -I${USR}/${MINGW32}/include -Idumbxinputemu
LIB_DIR32 = -L{USR}/${MINGW32}/lib/

GCC64 = ${BIN}/x86_64-w64-mingw32-gcc -D _WXI_MINGW
INCLUDE_DIR64 = -I${USR}/${MINGW64}/include -Idumbxinputemu
LIB_DIR64 = -L${USR}/${MINGW64}/lib/

.PHONY: default
default: all

# ---- Targets -------

%.32.o: %.c
	${GCC32} ${CFLAGS} ${INCLUDE_DIR32} ${LIB_DIR32} -c $? -o $@

%.64.o: %.c
	${GCC64} ${CFLAGS} ${INCLUDE_DIR64} ${LIB_DIR64} -c $? -o $@

# ------ 32 bit

build/32/xinput1_1.dll: xinput1_1/exports.def dumbxinputemu/dinput_input.32.o xinput1_1/xinput1_1.32.o
	mkdir -p build/32/
	${GCC32} --shared ${CFLAGS} ${INCLUDE_DIR32} ${LIB_DIR32} $? ${LIBS} -o $@

build/32/xinput1_2.dll: xinput1_2/exports.def dumbxinputemu/dinput_input.32.o xinput1_2/xinput1_2.32.o
	mkdir -p build/32/
	${GCC32} --shared ${CFLAGS} ${INCLUDE_DIR32} ${LIB_DIR32} $? ${LIBS} -o $@

build/32/xinput1_3.dll: xinput1_3/exports.def dumbxinputemu/dinput_input.32.o xinput1_3/xinput1_3.32.o
	mkdir -p build/32/
	${GCC32} --shared ${CFLAGS} ${INCLUDE_DIR32} ${LIB_DIR32} $? ${LIBS} -o $@

build/32/xinput1_4.dll: xinput1_4/exports.def dumbxinputemu/dinput_input.32.o xinput1_4/xinput1_4.32.o
	mkdir -p build/32/
	${GCC32} --shared ${CFLAGS} ${INCLUDE_DIR32} ${LIB_DIR32} $? ${LIBS} -o $@

build/32/xinput9_1_0.dll: xinput9_1_0/exports.def dumbxinputemu/dinput_input.32.o xinput9_1_0/xinput9_1_0.32.o
	mkdir -p build/32/
	${GCC32} --shared ${CFLAGS} ${INCLUDE_DIR32} ${LIB_DIR32} $? ${LIBS} -o $@

# ------ 64 bit
build/64/xinput1_1.dll: xinput1_1/exports.def dumbxinputemu/dinput_input.64.o xinput1_1/xinput1_1.64.o
	mkdir -p build/64/
	${GCC64} --shared ${CFLAGS} ${INCLUDE_DIR64} ${LIB_DIR64} $? ${LIBS} -o $@

build/64/xinput1_2.dll: xinput1_2/exports.def dumbxinputemu/dinput_input.64.o xinput1_2/xinput1_2.64.o
	mkdir -p build/64/
	${GCC64} --shared ${CFLAGS} ${INCLUDE_DIR64} ${LIB_DIR64} $? ${LIBS} -o $@

build/64/xinput1_3.dll: xinput1_3/exports.def dumbxinputemu/dinput_input.64.o xinput1_3/xinput1_3.64.o
	mkdir -p build/64/
	${GCC64} --shared ${CFLAGS} ${INCLUDE_DIR64} ${LIB_DIR64} $? ${LIBS} -o $@

build/64/xinput1_4.dll: xinput1_4/exports.def dumbxinputemu/dinput_input.64.o xinput1_4/xinput1_4.64.o
	mkdir -p build/64/
	${GCC64} --shared ${CFLAGS} ${INCLUDE_DIR64} ${LIB_DIR64} $? ${LIBS} -o $@

build/64/xinput9_1_0.dll: xinput9_1_0/exports.def dumbxinputemu/dinput_input.64.o xinput9_1_0/xinput9_1_0.64.o
	mkdir -p build/64/
	${GCC64} --shared ${CFLAGS} ${INCLUDE_DIR64} ${LIB_DIR64} $? ${LIBS} -o $@

clean:
	rm -f dumbxinputemu/*.o
	rm -f xinput1_1/*.o
	rm -f xinput1_2/*.o
	rm -f xinput1_3/*.o
	rm -f xinput1_4/*.o
	rm -f xinput9_1_0/*.o
	rm -f build/32/*
	rm -f build/64/*

32bit: build/32/xinput1_3.dll build/32/xinput9_1_0.dll

64bit: build/64/xinput1_3.dll build/64/xinput9_1_0.dll

all: 32bit
