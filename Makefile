CXX = g++
CXXFLAGS = -Wall

SRCDIR = src
BINDIR = bin

# Source files, corresponding object files and executable
SRCFILES = $(wildcard $(SRCDIR)/*.cpp)
OBJFILES = $(SRCFILES:$(SRCDIR)/%.cpp=$(BINDIR)/%.o)
EXE = $(BINDIR)/game

# Default rule
all: $(EXE)

# Sets appropriate flags for debug and release modes
release: CXXFLAGS += -O3
release: $(EXE)

debug: CXXFLAGS += -g -O0
debug: $(EXE)

# Executable rule dependant on all object files
$(EXE): $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o $(EXE) $(OBJFILES)

# Arbitrary object file rule dependant on its corresponding source file
$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.phony: all release debug