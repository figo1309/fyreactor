#make file 

include ./Rules.make


TARGET := fyreactor
PROJS	= ../lib/lib$(TARGET).a
######################

SRCDIR = ./src/net ./src/util ./src/xml ./src/json

VPATH :=  $(SRCDIR)  $(OBJDIR)

##########make 
##############################################################


all: $(PROJS) 
	
$(PROJS): $(OBJS)
	$(AR) $(PROJS)  $(PathOBJS) $(LibFLAGS) 

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
.cpp.o:
	$(CPP) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
	

clean:
	rm -f $(PROJS) $(PathOBJS)  $(PathOBJS:.o=.d)
	
