CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -pedantic -Wno-missing-field-initializers
LDFLAGS = -lglfw -lGLEW -lGL -lGLU
INFILES = 	main.cpp ImageData.cpp paint.cpp UndoSystem.cpp

OBJFILES = $(INFILES:.cpp=.o)

NAME = tinyPaint

all: $(NAME)

$(NAME):$(OBJFILES)
	$(CXX) $(CXXFLAGS) $(OBJFILES) $(LDFLAGS) -o $(NAME) 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJFILES)

fclean: clean
	rm -f $(NAME)

re: fclean all

# Additional targets for Exercise 04
run: $(NAME)
	./$(NAME)

dl:
	wget -O stb_image_write.h https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

.PHONY: all clean fclean re run download-stb install-deps