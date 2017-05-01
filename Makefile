TARGET=leaderboard.out

CXX:=clang++
CXX_FLAGS:=-g -O2 -std=c++14

INCLUDE_DIR:=./include
SRC_DIR:=./src
OBJ_DIR:=./build

INC:=-I$(INCLUDE_DIR) -I/usr/include -I/usr/local/include

LDFLAGS:=
LIBS:=-lamqpcpp

SRC:=$(wildcard $(SRC_DIR)/*.cpp)
SRC:=$(notdir $(SRC))

OBJS:=$(patsubst %.cpp,%.o,$(SRC))
OBJS:=$(addprefix $(OBJ_DIR)/,$(OBJS))

$(TARGET): $(OBJS)
	$(CXX) $(CXX_FLAGS) $(INC) $(LDFLAGS) $(LIBS) -o $@ $^

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) $(INC) -c $< -o $@

$(OBJS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(TARGET) $(OBJ_DIR)
