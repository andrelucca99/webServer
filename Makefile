# ==========================
# CONFIG
# ==========================
NAME        = webserv

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98

INCLUDES    = -I src/includes

SRC_DIR     = src
CONFIG_DIR  = $(SRC_DIR)/config
SERVER_DIR = $(SRC_DIR)/server
HTTP_DIR = $(SRC_DIR)/http
UTILS_DIR = $(SRC_DIR)/utils

SRCS        = \
	$(SRC_DIR)/main.cpp \
	$(CONFIG_DIR)/Tokenizer.cpp \
	$(CONFIG_DIR)/ConfigParser.cpp \
	$(SERVER_DIR)/Server.cpp \
	$(HTTP_DIR)/Router.cpp \
	$(HTTP_DIR)/HttpRequestParser.cpp \
	$(HTTP_DIR)/HttpResponse.cpp \
	$(HTTP_DIR)/CgiHandler.cpp \
	$(HTTP_DIR)/HttpError.cpp \
	$(UTILS_DIR)/File.cpp \

OBJS        = $(SRCS:.cpp=.o)

# Sources reusados pelo harness do parser (sem main.cpp / sem Server / sem
# Router-HTTP-Cgi). Compilados em arvore separada (.san.o) para nao colidir
# com objetos normais quando se alterna entre `make` e `make sanitize`.
PARSER_HARNESS_SRCS = \
	tests/parser_smoke.cpp \
	$(CONFIG_DIR)/Tokenizer.cpp \
	$(CONFIG_DIR)/ConfigParser.cpp \
	$(HTTP_DIR)/HttpRequestParser.cpp

# Flags de sanitizer (ASan engloba LeakSanitizer no Linux). -fno-sanitize-recover
# faz qualquer erro virar abort, garantindo exit code != 0.
SAN_FLAGS   = -fsanitize=address,undefined -fno-sanitize-recover=all \
	-fno-omit-frame-pointer -g -O1

NAME_SAN     = webserv_san
PARSER_SMOKE = parser_smoke

SAN_OBJS         = $(SRCS:%.cpp=%.san.o)
PARSER_SAN_OBJS  = $(PARSER_HARNESS_SRCS:%.cpp=%.san.o)

# ==========================
# RULES
# ==========================

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- Sanitizer build ---
# webserv_san: mesmo binario, recompilado com ASan/UBSan. Usado pelo
# tests/leak_check.sh para subir o servidor instrumentado.
sanitize: $(NAME_SAN) $(PARSER_SMOKE)

$(NAME_SAN): $(SAN_OBJS)
	$(CXX) $(CXXFLAGS) $(SAN_FLAGS) $(SAN_OBJS) -o $(NAME_SAN)

$(PARSER_SMOKE): $(PARSER_SAN_OBJS)
	$(CXX) $(CXXFLAGS) $(SAN_FLAGS) $(PARSER_SAN_OBJS) -o $(PARSER_SMOKE)

%.san.o: %.cpp
	$(CXX) $(CXXFLAGS) $(SAN_FLAGS) $(INCLUDES) -c $< -o $@

# Roda parser_smoke + suite HTTP sob sanitizers.
leak-check: sanitize
	bash tests/leak_check.sh

# --- Valgrind build ---
# parser_smoke_vg: parser harness sem sanitizers (sanitizers + valgrind nao
# se combinam). webserv "normal" (alvo all) ja serve para o valgrind.
PARSER_SMOKE_VG = parser_smoke_vg
VG_FLAGS        = -g -O0
PARSER_VG_OBJS  = $(PARSER_HARNESS_SRCS:%.cpp=%.vg.o)

valgrind-build: $(NAME) $(PARSER_SMOKE_VG)

$(PARSER_SMOKE_VG): $(PARSER_VG_OBJS)
	$(CXX) $(CXXFLAGS) $(VG_FLAGS) $(PARSER_VG_OBJS) -o $(PARSER_SMOKE_VG)

%.vg.o: %.cpp
	$(CXX) $(CXXFLAGS) $(VG_FLAGS) $(INCLUDES) -c $< -o $@

valgrind-check: valgrind-build
	bash tests/valgrind_check.sh

clean:
	rm -f $(OBJS) $(SAN_OBJS) $(PARSER_SAN_OBJS) $(PARSER_VG_OBJS)

fclean: clean
	rm -f $(NAME) $(NAME_SAN) $(PARSER_SMOKE) $(PARSER_SMOKE_VG)

re: fclean all

# ==========================
# PHONY
# ==========================

.PHONY: all clean fclean re sanitize leak-check valgrind-build valgrind-check
