CXX = clang++
CXXFLAGS = -std=c++23
TARGET = shell
SRCS = src/main.cpp src/ls.cpp src/parse.cpp src/execute.cpp src/config.cpp src/git.cpp src/startup.cpp src/builtins.cpp src/process.cpp

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
    FLAGS = -L/opt/homebrew/opt/readline/lib -I/opt/homebrew/opt/readline/include -lreadline
else
    FLAGS = -lreadline
endif

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) -Iinclude $(FLAGS)

install: all
	cp $(TARGET) /usr/local/bin/mysh

uninstall:
	rm -f /usr/local/bin/mysh

clean:
	rm -f $(TARGET)
