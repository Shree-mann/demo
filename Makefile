# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2

# Source files and object files for each target
SRCS_WIFI4 = wifi4.cpp
SRCS_WIFI5 = wifi5.cpp
SRCS_WIFI6 = wifi6.cpp

OBJS_WIFI4 = $(SRCS_WIFI4:.cpp=.o)
OBJS_WIFI5 = $(SRCS_WIFI5:.cpp=.o)
OBJS_WIFI6 = $(SRCS_WIFI6:.cpp=.o)

# Target executables
TARGET_WIFI4 = wifi4
TARGET_WIFI5 = wifi5
TARGET_WIFI6 = wifi6

# Default target (compile all versions)
all: $(TARGET_WIFI4) $(TARGET_WIFI5) $(TARGET_WIFI6)

# Build targets for each executable
$(TARGET_WIFI4): $(OBJS_WIFI4)
	$(CXX) $(CXXFLAGS) -o $(TARGET_WIFI4) $(OBJS_WIFI4)

$(TARGET_WIFI5): $(OBJS_WIFI5)
	$(CXX) $(CXXFLAGS) -o $(TARGET_WIFI5) $(OBJS_WIFI5)

$(TARGET_WIFI6): $(OBJS_WIFI6)
	$(CXX) $(CXXFLAGS) -o $(TARGET_WIFI6) $(OBJS_WIFI6)

# Rule for building object files for each source
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target to remove all object files and executables
clean:
	rm -f $(OBJS_WIFI4) $(OBJS_WIFI5) $(OBJS_WIFI6) $(TARGET_WIFI4) $(TARGET_WIFI5) $(TARGET_WIFI6)

# Run targets for each executable
run_wifi4: $(TARGET_WIFI4)
	./$(TARGET_WIFI4)

run_wifi5: $(TARGET_WIFI5)
	./$(TARGET_WIFI5)

run_wifi6: $(TARGET_WIFI6)
	./$(TARGET_WIFI6)

# Phony targets
.PHONY: all clean run_wifi4 run_wifi5 run_wifi6
