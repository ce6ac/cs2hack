CXX=g++
CXXFLAGS=-I./memflow/memflow-win32-ffi/ -I./memflow/memflow-ffi/ -I./include -L./memflow/target/release -Wno-multichar
LIBS=-lm -Wl,--no-as-needed -ldl -lpthread -l:libmemflow_win32_ffi.a -lcurl -lfmt

OUTDIR=./build
OBJDIR=$(OUTDIR)/obj

$(shell mkdir -p $(OBJDIR))
$(shell cp memflow/memflow-qemu-procfs/target/release/libmemflow_qemu_procfs.so $(OUTDIR))

%.o: %.cpp
	$(CXX) -c -o $(OBJDIR)/$@ $< $(CXXFLAGS)

qmp.o: include/qmp.cpp
	$(CXX) -c -o $(OBJDIR)/$@ $< $(CXXFLAGS)

cs2hack: main.o memory.o comms.o game.o qmp.o offsets.o
	$(CXX) -o $(OUTDIR)/$@ $(OBJDIR)/main.o $(OBJDIR)/memory.o $(OBJDIR)/comms.o $(OBJDIR)/game.o $(OBJDIR)/qmp.o $(OBJDIR)/offsets.o $(CXXFLAGS) $(LIBS)

.PHONY: all
all: cs2hack

.DEFAULT_GOAL := all

clean:
	rm -rf $(OUTDIR)
