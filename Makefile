#
# Makefile for final project Computer Graphics
# Sebastiaan Alvarez Rodriguez
#

CC            ?= gcc 
WARNINGS       = -Wall -Wextra -pedantic
# OTHERS         = 

BASESRCDIR     = src

SERVERSRCDIR   = $(BASESRCDIR)/server
SERVERSRCFILES = $(notdir $(shell find $(SERVERSRCDIR)/*.c))
SERVERSRCS     = $(addprefix $(SERVERSRCDIR),$(SERVERSRCFILES))

CLIENTSRCDIR   = $(BASESRCDIR)/client
CLIENTSRCFILES = $(notdir $(shell find $(CLIENTSRCDIR)/*.c))
CLIENTSRCS     = $(addprefix $(CLIENTSRCDIR),$(CLIENTSRCFILES))

BASEOBJDIR     = obj

SERVEROBJDIR   = $(BASEOBJDIR)/server/
SERVEROBJFILES = $(SERVERSRCFILES:.c=.o)
SERVEROBJS     = $(addprefix $(SERVEROBJDIR),$(SERVEROBJFILES))

CLIENTOBJDIR   = $(BASEOBJDIR)/client/
CLIENTOBJFILES = $(CLIENTSRCFILES:.c=.o)
CLIENTOBJS     = $(addprefix $(CLIENTOBJDIR),$(CLIENTOBJFILES))

LIBDIR         = lib
IDIRS          = -I$(BASESRCDIR)

CFLAGS         = $(IDIRS) -std=c99 $(WARNINGS) -lm -lasound

SERVEREXEC     = server
CLIENTEXEC     = client
MAKEFLAGS      = -j

.PHONY: all server client clean

all: server client

server: $(SERVEROBJS) 
	$(CC) $(SERVEROBJS) -o $(SERVEREXEC) $(CFLAGS)

client: $(CLIENTOBJS)
	$(CC) $(CLIENTOBJS) -o $(CLIENTEXEC) $(CFLAGS)

clean:
	rm -rf $(BASEOBJDIR) $(SERVEREXEC) $(CLIENTEXEC)

$(SERVEROBJDIR)%.o: $(SERVERSRCDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(CLIENTOBJDIR)%.o: $(CLIENTSRCDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(SERVEROBJS): | $(SERVEROBJDIR)

$(SERVEROBJDIR):
	mkdir -p $(SERVEROBJDIR)

$(CLIENTOBJS): | $(CLIENTOBJDIR)

$(CLIENTOBJDIR):
	mkdir -p $(CLIENTOBJDIR)

c: clean

.PHONY: c
