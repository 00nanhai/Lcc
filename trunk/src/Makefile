
PRO_VERSION +=
UPDATE_TIME +=
SVN_VERSION +=
COMPLIE_TIME=$(shell date +%Y/%m/%d\ %H:%M:%S)
MACROS += -DSVN_VERSION=\"$(SVN_VERSION)\"
MACROS += -DUPDATE_TIME=\"$(UPDATE_TIME)\"
MACROS += -DPRO_VERSION=\"$(PRO_VERSION)\"
MACROS += -DCOMPLIE_TIME=\""$(COMPLIE_TIME)"\"

#CC=gcc
CXX=g++
#CFLAGS= -ggdb -g3 -Wall
#CFLAGS= -ggdb -O2 -Wall
CFLAGS= -ggdb -O3 -Wall

MACROS +=#-D_SW_DEBUG_
LIBS = /usr/local/lib/libxLog.a /usr/lib64/mysql/libmysqlclient.a \
		 -lz -lpthread \
	   -lcrypt -lnsl -lm -lrt -lssl -lcrypto \
		 -L../lib/libevent/ -levent -L../lib/libevent/ -levent_pthreads -L../lib/libevent/#-lboost
INCLUDE = -I ./include/ -I../public/ -I../public/slib/ -I../public/md5/ \
		  -I../include/
CFLAGS += $(INCLUDE) $(MACROS)

vpath %.c ./
vpath %.cpp ./
vpath %.c ../public/
vpath %.cpp ../public/
vpath %.c ../public/slib/
vpath %.cpp ../public/slib/
vpath %.cpp ../public/md5/

OBJS = config.o json.o path.o slock.o slog.o smutex.o utils.o xxtea.o my_md5.o \
	   file_opt.o process_opt.o sw_cond_mutex.o sw_flock.o sw_pthread_signal.o \
	   create_A.o  mydb_instance.o create_B.o main.o mysql_worker.o  request.o \
	   evhttp_opt.o response.o host_dump.o

TARGET +=lcc

$(TARGET) : $(OBJS)
	$(CXX) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
%.o : %.c
	$(CXX) $(CFLAGS)  -o $@ -c $<
%.o : %.cpp
	$(CXX) $(CFLAGS)  -o $@ -c $<

.PHONY:clean
clean :
	rm -rf $(TARGET) $(OBJS)

