
export TCL_INCLUDE_HOME=/usr/include/tcl8.6
# export C_INCLUDE_PATH=$(TCL_INCLUDE_HOME):$(C_INCLUDE_PATH)
# export CPLUS_INCLUDE_PATH=$(TCL_INCLUDE_HOME):$(CPLUS_INCLUDE_PATH)

all: clean compile



compile:
	g++ -g main.cpp -o test  -I$(TCL_INCLUDE_HOME) -ltcl8.6 -ltk8.6 -lpthread -fpermissive -lncurses



clean:
	rm -f *.log test



