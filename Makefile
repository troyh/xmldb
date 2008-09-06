CXXFLAGS=-g -I/usr/include/libxml++-2.6 -I/usr/lib/libxml++-2.6/include -I/usr/include/libxml2 -I/usr/include/glibmm-2.4 -I/usr/lib/glibmm-2.4/include -I/usr/include/sigc++-2.0 -I/usr/lib/sigc++-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  -lxml++-2.6 -lxml2 -lglibmm-2.4 -lgobject-2.0 -lsigc-2.0 -lglib-2.0

all: index kvpairs split query

index: index.o XMLDoc.o
	g++ -o $@ $^ -lxml++-2.6 -lxml2 -lxqilla -lboost_filesystem -lboost_serialization

kvpairs: kvpairs.o XMLDoc.o
	g++ -o $@ $^ -lxml++-2.6 -lxml2 -lxqilla -lboost_filesystem
	
split: split.o
	g++ -o $@ $^ -lxml++-2.6 -lxml2 -lxqilla

query: query.o
	g++ -o $@ $^ -lxml++-2.6 -lxml2 -lboost_filesystem -lboost_serialization

XMLDoc.o: XMLDoc.cc XMLDoc.hpp

index.o: index.cc XMLDoc.hpp

clean:
	rm -f index kvpairs split *.o