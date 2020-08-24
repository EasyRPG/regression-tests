#For local build configuration MinGW, Dynrpg, etc..
-include local.mk
DYNRPG ?= dynrpg
CC ?= gcc.exe
CXX ?= g++.exe

CPPFLAGS=-D_GLIBCXX_USE_CXX11_ABI=0 -I$(DYNRPG)/include -Isubhook
CXXFLAGS=-std=c++11
LDFLAGS=-L$(DYNRPG)/lib -lDynRPG

easyrpg-regtest.dll: main.o subhook/subhook.o
	$(CXX) -shared -o $@ $^ -Wl,--out-implib,lib$(basename $@).a $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $^

subhook/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $^

all: easyrpg-regtest.dll

clean:
	rm -f *.a || del *.a
	rm -f *.dll || del *.dll
	rm -f *.o subhook/*.o || del *.o subhook/*.o

install: all
	install easyrpg-regtest.dll $(DESTDIR)
