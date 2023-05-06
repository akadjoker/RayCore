
LIBPATH = libs

CXX = g++

CXXFLAGS =-I$(LIBPATH)/chipmunk/include/ -I$(LIBPATH)/box2d/include/box2d/ -I$(LIBPATH)/lua-5.5.5/src  -I$(LIBPATH)/raylib/src  -DPLATFORM_DESKTOP -std=c++11 
CXXFLAGS += -O3
#CXXFLAGS +=-fsanitize=address -g #-fsanitize=undefined -fno-omit-frame-pointer -g
LIBS =-Llib -lsraylib -llua -lbox2d -lm


SRCDIR = src
OBJDIR = obj

SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))


TARGET = core

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	./$(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(TARGET)