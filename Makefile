
export TCL_INCLUDE_HOME=/usr/include/tcl8.6
export ARGPARSE_INCLUDE_HOME=./argparse-cpp/src/argparse
#export LD_LIBRARY_PATH=/opt/tcl_9.1/lib:/opt/tk_9.1/lib
#export TCL_INCLUDE_HOME=/opt/tcl_9.1/include
# export C_INCLUDE_PATH=$(TCL_INCLUDE_HOME):$(C_INCLUDE_PATH)
# export CPLUS_INCLUDE_PATH=$(TCL_INCLUDE_HOME):$(CPLUS_INCLUDE_PATH)

all: clean compile



compile:
	g++-10 -std=c++17 -g main.cpp -o test  -I$(TCL_INCLUDE_HOME)  -ltcl8.6 -ltk8.6 -lpthread -fpermissive -lncurses
	#g++ -g main.cpp -o test  -I$(TCL_INCLUDE_HOME) -I$(ARGPARSE_INCLUDE_HOME) -ltcl9.1 -ltcl9tk9.1 -lpthread -fpermissive -lncurses



clean:
	rm -f *.log test



