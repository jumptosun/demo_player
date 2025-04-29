.PHONY: all clean

SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp, %, ${SRC})

CC=gcc
CXX=g++
CXXFLAGS=-g -c -O0 -fPIC -I../ -I/usr/include/x86_64-linux-gnu
LDFLAGS=-Wl,--no-as-needed  -lavformat -lavcodec -lavutil -pthread 

all: ${OBJ}

dpl_log.o: ../src/dpl_log.cpp
	${CXX} ${CXXFLAGS} ../src/dpl_log.cpp

demuxer.o: ../src/demuxer.cpp
	${CXX} ${CXXFLAGS} ../src/demuxer.cpp

dpl_log_test: dpl_log.o dpl_log_test.o

dpl_demuxer_test: dpl_log.o demuxer.o 
	${CXX} ${CXXFLAGS} dpl_demuxer_test.cpp
	${CXX} dpl_log.o demuxer.o dpl_demuxer_test.o ${LDFLAGS} -o dpl_demuxer_test


dpl_decoder_test: ../src/demuxer.cpp ../src/decoder.cpp ../src/dpl_log.cpp

alsa_test: 

clock_reference_test:  ../src/clock_reference.cpp

audio_render_test:  ../src/audio_render.cpp ../src/dpl_log.cpp  ../src/clock_reference.cpp ../src/dpl_utility.cpp

lockfree_queue_test:

libav_link_test: 
	${CC} -c -fPIC libav_link_test.c
	${CC} libav_link_test.o ${LDFLAGS} -o libav_link_test.exe

clean:
	rm ${OBJ}
