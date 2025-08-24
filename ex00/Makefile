CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -pedantic
LDFLAGS = -lGL -lGLU -lglut -lGLEW
INFILES = main.cpp
OBJFILES = $(INFILES:.cpp=.o)
NAME = tinyPaint

all: $(NAME)

$(NAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(OBJFILES) $(LDFLAGS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJFILES)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re