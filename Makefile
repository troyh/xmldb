CXXFLAGS=-g -rdynamic -I/usr/local/include/boost-1_36/ -I/usr/include/libxml++-2.6 -I/usr/lib/libxml++-2.6/include -I/usr/include/libxml2 -I/usr/include/glibmm-2.4 -I/usr/lib/glibmm-2.4/include -I/usr/include/sigc++-2.0 -I/usr/lib/sigc++-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  -lxml++-2.6 -lxml2 -lglibmm-2.4 -lgobject-2.0 -lsigc-2.0 -lglib-2.0

%.o : %.cc
	$(CC) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@


.PHONY: clean FORCE

all: odeps.mk ouzo index kvpairs split query

odeps.mk: *.cc *.hpp
	$(CC) -MM *.cc > $@

ouzo: cli.o libouzo.a
	$(CC) -g -rdynamic -o $@ $^ -lxml++-2.6 -lxml2 -lxqilla -lboost_filesystem-gcc43-mt -lboost_serialization-gcc43-mt -L. -louzo -liberty

index: index.o XMLDoc.o
	$(CC) -o $@ $^ -lxml++-2.6 -lxml2 -lxqilla -lboost_filesystem-gcc43-mt -lboost_serialization-gcc43-mt

kvpairs: kvpairs.o XMLDoc.o
	$(CC) -o $@ $^ -lxml++-2.6 -lxml2 -lxqilla -lboost_filesystem-gcc43-mt
	
split: split.o
	$(CC) -o $@ $^ -lxml++-2.6 -lxml2 -lxqilla

query: query.o
	$(CC) -o $@ $^ -lxml++-2.6 -lxml2 -lboost_filesystem-gcc43-mt -lboost_serialization-gcc43-mt

libouzo.a: Ouzo.o DocSet.o Index.o StringIndex.o UIntIndex.o Config.o DocumentBase.o Exception.o QueryTree.o XRefTable.o Keys.o
	rm -f $@
	ar rcs $@ $^

include odeps.mk

clean:
	rm -f index kvpairs split ouzo *.o