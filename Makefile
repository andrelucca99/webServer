# ==========================
# CONFIG
# ==========================
NAME        = webserv_test

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98

INCLUDES    = -I src/includes

SRC_DIR     = src
CONFIG_DIR  = $(SRC_DIR)/config
SERVER_DIR  = $(SRC_DIR)/server
HTTP_DIR    = $(SRC_DIR)/http

SRCS        = \
	$(SRC_DIR)/main.cpp \
	$(CONFIG_DIR)/Tokenizer.cpp \
	$(CONFIG_DIR)/ConfigParser.cpp \
	$(SERVER_DIR)/Server.cpp \
	$(SERVER_DIR)/Socket.cpp \
	$(HTTP_DIR)/HttpRequestParser.cpp \
	$(HTTP_DIR)/HttpResponse.cpp

OBJS        = $(SRCS:.cpp=.o)

# ==========================
# TEST PARSER
# ==========================
TEST_NAME   = test_parser

TEST_SRCS   = \
	$(HTTP_DIR)/test_parser.cpp \
	$(HTTP_DIR)/HttpRequestParser.cpp \
	$(HTTP_DIR)/HttpResponse.cpp

TEST_OBJS   = $(TEST_SRCS:.cpp=.o)

# ==========================
# RULES
# ==========================

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(TEST_NAME): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) -o $(TEST_NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TEST_OBJS)

fclean: clean
	rm -f $(NAME) $(TEST_NAME)

re: fclean all

# ==========================
# PHONY
# ==========================

.PHONY: all clean fclean re