CC      ?= gcc
SRC      = src
OBJS     = obj

WARNINGS = -Wall -Wextra -Wno-unused-variable -pedantic -g
IDIRS    = -I$(SRC) -I$(SRC)/communication
LDIRS    =  -lm -lasound
CFLAGS   = $(IDIRS) -std=gnu99 $(WARNINGS) $(LDIRS)

find = $(shell find $1 -type f ! -path $3 -name $2 -print 2>/dev/null)

SERVERSRCS := $(call find, $(SRC)/, "*.c", "*/client/*")
CLIENTSRCS := $(call find, $(SRC)/, "*.c", "*/server/*")
SERVEROBJECTS := $(SERVERSRCS:%.c=$(OBJS)/%.o)
CLIENTOBJECTS := $(CLIENTSRCS:%.c=$(OBJS)/%.o)

CLEAR  = [0m
CYAN   = [1;36m
GREEN  = [1;32m
YELLOW = [1;33m
WHITE  = [1;37m

xoutofy = $(or $(eval PROCESSED := $(PROCESSED) .),$(info $(WHITE)[$(YELLOW)$(words $(PROCESSED))$(WHITE)] $1$(CLEAR)))

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

git: clean
	git add *
	git commit
	git push

c: clean

.PHONY: c clean
