# Makefile for the ELIS project

# Authors:
# Felipe Ramos
# Daniel Guerra
#
# Makefile conventions
SHELL = /bin/sh

# Directories
INCDIR = ./include
SRCDIR = ./src
OBJDIR = ./obj
BINDIR = ./bin
DATADIR = ./data
# DOCDIR = ./Documentation

# Macros
CC = g++
CFLAGS = -Wall -w -g -ggdb -std=c++11 -I. -I$(INCDIR) -lncurses
RM = -rm
PROJ_NAME = elis
# DOC_NAME = index.html

HEADERS := $(wildcard $(INCDIR)/*)
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: project

project: $(OBJECTS) $(HEADERS) | $(BINDIR)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(BINDIR)/$(PROJ_NAME)
	@ln -sfv $(BINDIR)/$(PROJ_NAME) $(PROJ_NAME)

docs:
	@mkdir -p $(DOCDIR)
	@doxygen config
	@ln -sfv $(DOCDIR)/html/index.html $(DOC_NAME)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(BINDIR):
	@mkdir -p $(BINDIR)

# Clean PHONY targets
.PRONY: clean clean_proj clean_doc

clean: clean_proj clean_doc

clean_proj:
	@echo "Removing OBJDIR..."
	@$(RM) -r $(OBJDIR)
	@echo "Removing BINDIR..."
	@$(RM) -r $(BINDIR)
	@echo "Removing symlink..."
	@$(RM) -f $(PROJ_NAME)
	@echo "Clean-up completed!"

clean_doc:
	@echo "Removing documentation files..."
	@$(RM) -r Documentation/
	@$(RM) -f $(DOC_NAME) 
