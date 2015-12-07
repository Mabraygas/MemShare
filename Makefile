OTH_PATH = ./3rd_party

AR      = ar 
ARFLAGS = -cr

CXX      = g++
INC      = -I./include 							\
		   -I$(OTH_PATH)/eagle/include  		\
		   -I$(OTH_PATH)/eagle_tool/include     \
		   -I$(OTH_PATH)/log4plus/include 		\
		   -I$(OTH_PATH)/urcu/include   		\
		   -I$(OTH_PATH)/finger/include         \
		   -I$(OTH_PATH)/log/include 			\
		   -I$(OTH_PATH)/client_socket/include 	\
		   $(NULL)
		   
CXXFLAGS = $(INC) -static -Wall -g


# makefile的缺省目标
target = libmemshare.a
.PHONY : all 
all : $(target)
	rm -rf lib/*.a
	mv $(target) lib

sources = src/memshare.cc 					\
		  src/memshare_global.cc 			\
		  src/memshare_recv_work.cc 		\
		  src/memshare_handle_work.cc 		\
		  src/memshare_send_work.cc 		\
		  src/memshare_share_work.cc

objs := $(sources:%.cc=%.o)

# 目标文件的依赖及生成的命令
$(target) : $(objs)
	$(AR) $(ARFLAGS) $@ $^

# .o文件的依赖及生成的命令
%.o : %.cc
	$(CXX) -c $< -o $@ $(CXXFLAGS)

.PHONY : clean
clean :
	find ./ -name '*~'  -exec rm -rf {} \;
	find ./ -name '*.o' -exec rm -rf {} \;

