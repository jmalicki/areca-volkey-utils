.PHONY: all
default: all ;

# global
APIROOT=20200420-383
BIN=$(APIROOT)/bin
OS=linux

APIURL="https://www.areca.us/support/download/API/API_code.zip"
APIZIP=API_code.zip

PM=x86_64
CFG=release
APPOUTDIR=build
PLATFORMDIR=$(BIN)/$(OS)/$(PM)
OUTDIR=$(APPOUTDIR)/$(CFG)

# output filename
LIBFILE=libarclib64.so
OUTFILE=$(OUTDIR)/areca-volkey
GCCMOPT = -m64

PUBLIC_INC=$(APIROOT)/include
INCLUDE=-I"$(PUBLIC_INC)"

LIB=$(PLATFORMDIR)/release/$(LIBFILE)
COMMON_OBJ=$(OUTDIR)/areca-volkey.o
OBJ=$(COMMON_OBJ)

COMPILE=g++ -g $(GCCMOPT) -c  -D_REENTRANT $(INCLUDE) -o "$(OUTDIR)/$(*F).o" "$<"
LINK=g++ -g $(GCCMOPT) -o "$(OUTFILE)" $(OBJ) $(LIB) -lpthread -lboost_program_options

# Build rules
all: $(OUTFILE)

# Pattern rules
$(OUTDIR)/%.o : %.cpp $(APIROOT)
	$(COMPILE)

$(APIZIP):
	curl $(APIURL) -o $(APIZIP)

$(APIROOT): $(APIZIP)
	unzip -u $(APIZIP)
	# to bump timestamp for make
	touch $(APIROOT)

$(OUTFILE): $(OUTDIR) $(OBJ)
	$(LINK)

$(OUTDIR):
	mkdir -p "$(OUTDIR)"

# Rebuild this project
rebuild: cleanall all

# Clean this project
clean:
	rm -f $(OUTFILE) $(OBJ) $(APIZIP)
	rm -rf $(APIROOT) $(APPOUTDIR)

# Clean this project and all dependencies
cleanall: clean
