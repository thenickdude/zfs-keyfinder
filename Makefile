CXXFLAGS = -Wall -O3

all : zfs-keyfinder

zfs-keyfinder : zfs-keyfinder.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean : 
	rm -f zfs-keyfinder