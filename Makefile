# ==========================
# CONFIG
# ==========================
NAME        = webserv_test

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98

INCLUDES    = -I src/includes

SRC_DIR     = src
CONFIG_DIR  = $(SRC_DIR)/config

SRCS        = \
	$(SRC_DIR)/main.cpp \
	$(CONFIG_DIR)/Tokenizer.cpp \
	$(CONFIG_DIR)/ConfigParser.cpp

OBJS        = $(SRCS:.cpp=.o)

# ==========================
# RULES
# ==========================

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

# ==========================
# PHONY
# ==========================

.PHONY: all clean fclean re