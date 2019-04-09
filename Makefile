CC      ?= gcc
SRC      = src
OBJS     = obj

WARNINGS = -Wall -Wextra -pedantic -g
IDIRS    = -I$(SRC)
LDIRS    =  -lm -lasound
CFLAGS   = $(IDIRS) -std=gnu99 $(WARNINGS) $(LDIRS)

find = $(shell find $1 -type f ! -name 'server.c' ! -name 'client.c' -name $2 -print 2>/dev/null)

SRCS := $(call find, $(SRC)/, "*.c")
SERVEROBJECTS := $(SRCS:%.c=$(OBJS)/%.o) $(OBJS)/$(SRC)/server/server.o
CLIENTOBJECTS := $(SRCS:%.c=$(OBJS)/%.o) $(OBJS)/$(SRC)/client/client.o

CLEAR  = [0m
CYAN   = [1;36m
GREEN  = [1;32m
YELLOW = [1;33m
WHITE  = [1;37m

xoutofy = $(or $(eval PROCESSED := $(PROCESSED) .),$(info $(WHITE)[$(YELLOW)$(words $(PROCESSED))$(WHITE)/$(YELLOW)$(words $(SRCS) . . )$(WHITE)] $1$(CLEAR)))

.PHONY: server client git

all: server client

server: $(SERVEROBJECTS)
	@$(call xoutofy,$(GREEN)Linking $@)
	$(CC) $(SERVEROBJECTS) -o $@ $(CFLAGS)

client: $(CLIENTOBJECTS)
	@$(call xoutofy,$(GREEN)Linking $@)
	$(CC) $(CLIENTOBJECTS) -o $@ $(CFLAGS)

$(OBJS)/%.o: %.c
	@$(call xoutofy,$(CYAN)Compiling $<)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@echo Cleaning...
	@rm -rf $(OBJS) server client

git:
	git add *
	git commit
	git push

c: clean

.PHONY: c clean
