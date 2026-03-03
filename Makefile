CXX = g++
TARGET = shell
SRCS = main.cpp ls.cpp parse.cpp execute.cpp config.cpp git.cpp startup.cpp builtins.cpp

# Betriebssystem erkennen
UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
    FLAGS = -L/opt/homebrew/opt/readline/lib -I/opt/homebrew/opt/readline/include -lreadline
else
    FLAGS = -lreadline
endif

all:
	$(CXX) $(SRCS) -o $(TARGET) $(FLAGS)

install: all
	cp $(TARGET) /usr/local/bin/mysh

uninstall:
	rm -f /usr/local/bin/mysh

clean:
	rm -f $(TARGET)
