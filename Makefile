CXX = g++
FLAGS = -L/opt/homebrew/opt/readline/lib -I/opt/homebrew/opt/readline/include -lreadline
SRCS = main.cpp ls.cpp parse.cpp execute.cpp config.cpp git.cpp
TARGET = shell

all:
	$(CXX) $(SRCS) -o $(TARGET) $(FLAGS)

clean:
	rm -f $(TARGET)
