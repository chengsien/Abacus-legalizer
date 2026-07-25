SRC_DIR := ./src/
################################################################################

SRCS := $(wildcard $(SRC_DIR)*.cpp)
OBJS := $(patsubst $(SRC_DIR)%.cpp, $(SRC_DIR)%.o, $(SRCS))
################################################################################

TARGET = HW4_M16144122 # 學號

CXX = g++
CXXFLAGS = -O3 -Wall -g -std=c++11
LIB =
INCLUDE =

#################################################################################

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIB) -o $(TARGET)

$(SRC_DIR)%.o: $(SRC_DIR)%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(TARGET) $(SRC_DIR)*.o core *~

