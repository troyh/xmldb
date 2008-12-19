OBJS=objs
CXXFLAGS=-g -I.. -I/usr/local/include/boost-1_36/ -I/usr/include/libxml++-2.6 -I/usr/lib/libxml++-2.6/include -I/usr/include/libxml2 -I/usr/include/glibmm-2.4 -I/usr/lib/glibmm-2.4/include -I/usr/include/sigc++-2.0 -I/usr/lib/sigc++-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  -lxml++-2.6 -lxml2 -lglibmm-2.4 -lgobject-2.0 -lsigc-2.0 -lglib-2.0

$(OBJS)/%.o : ../%.cc
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@
$(OBJS)/%.o : %.cc
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@


.PHONY: clean FORCE all

all: odeps.mk indexes docbases

odeps.mk: *.cc
	gcc -MM -I.. $^ | sed -e "s/^\\(.*\\).o:/objs\/\\1.o:/" > $@

indexes: $(OBJS)/indexes.o
	$(CC) -o $@ $^ -lxml++-2.6 -lxml2 -lxqilla -lboost_filesystem-gcc43-mt -lboost_serialization-gcc43-mt -L.. -louzo -liberty

docbases: $(OBJS)/docbases.o
	$(CC) -o $@ $^ -lxml++-2.6 -lxml2 -lxqilla -lboost_filesystem-gcc43-mt -lboost_serialization-gcc43-mt -L.. -louzo -liberty

include odeps.mk

clean:
	rm -f indexes $(OBJS)/*.o
