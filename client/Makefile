CXX = g++

DEPS = ./deps
OUTDIR = ./build
OBJDIR = $(OUTDIR)/obj

CPPFLAGS = -I$(DEPS)/memflow/memflow-win32-ffi/ \
           -I$(DEPS)/memflow/memflow-ffi/ \
           -I./include \
           -I$(DEPS)/ixwebsocket \
           -I$(DEPS)/curl/build/include

CXXFLAGS = -Wno-multichar

LDFLAGS = -L$(DEPS)/memflow/target/release \
          -L$(DEPS)/ixwebsocket/build \
          -L$(DEPS)/curl/build/lib

LIBS = -lm -Wl,--no-as-needed -ldl -lpthread \
       -l:libmemflow_win32_ffi.a \
       -l:libcurl.a \
       -lpsl \
       -lzstd \
       -lbrotlidec \
       -lbrotlicommon \
       -lfmt \
       -lixwebsocket \
       -lz \
       -lssl \
       -lcrypto

OBJS = $(OBJDIR)/main.o \
       $(OBJDIR)/memory.o \
       $(OBJDIR)/comms.o \
       $(OBJDIR)/game.o \
       $(OBJDIR)/qmp.o \
       $(OBJDIR)/offsets.o

.PHONY: all clean
.DEFAULT_GOAL := all

all: cs2hack

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/main.o: main.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/memory.o: memory.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/comms.o: comms.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/game.o: game.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/qmp.o: include/qmp.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/offsets.o: offsets.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

cs2hack: $(OBJS)
	$(CXX) -o $(OUTDIR)/$@ $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	rm -rf $(OUTDIR)