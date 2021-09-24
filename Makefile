# Output file basename
OUTPUT	= App
# Output directories
BIN		= bin
OBJ		= obj

#-------------------------------------------------------------------------------
#		Tools
#-------------------------------------------------------------------------------
# Doxygen tools

DOC 	= doc
HTML 	= $(DOC)/html/
DOXYFILE= $(DOC)/Doxyfile 
BROWSER	= firefox

# Compilation tools
CC		= gcc
SIZE	= size
STRIP	= strip
OBJCOPY	= objcopy
DEBUGGER= gdb

# Flags
INCLUDES	= -Isrc/

OPTIMIZATION	= -O0
CFLAGS	= $(OPTIMIZATION) -Wall -Wextra -g3 -DUSE_INPUT_FILE -DSPECTRUM_binning
LDFLAGS = $(OPTIMIZATION) -Wall -Wextra -g3
LDLIBS = -lm
#-------------------------------------------------------------------------------
#		FILES
#-------------------------------------------------------------------------------

# Directories where source files can be found
VPATH    = src/
VPATH   += src/spectrum/
VPATH	+= src/spectrum/array_test_data
VPATH   += src/InputOutput/

# Objects build from C source files
C_OBJECTS	 = main.o
C_OBJECTS	 += spectrum.o
C_OBJECTS    += sample.o
C_OBJECTS	 += InputOutput.o


#Append OBJ and BIN directories to output filename
OUTPUT_BIN	= $(BIN)/$(OUTPUT).bin

#-------------------------------------------------------------------------------
#		Rules
#-------------------------------------------------------------------------------
all: $(BIN) $(OBJ) $(OUTPUT)

$(BIN) $(OBJ):
	mkdir $@

C_OBJECTS_OBJ = $(addprefix $(OBJ)/, $(C_OBJECTS))

$(OUTPUT): $(C_OBJECTS_OBJ) $(LDLIBS)
	@echo "Compilation complete"
	@$(CC) $(LDFLAGS) -o $(OUTPUT_BIN) $^
	@echo "Linking complete"
	@echo "Following, the size of objects and binary"
	@$(SIZE) $(C_OBJECTS_OBJ) $(OUTPUT_BIN)

$(C_OBJECTS_OBJ): $(OBJ)/%.o: %.c Makefile $(OBJ) $(BIN)
	@$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<


debug: $(OUTPUT)
	@echo "Starting debug session of $(OUTPUT_BIN)"
	@$(DEBUGGER) $(OUTPUT_BIN)

.PHONY: clean run doc print_doc

run:
	@echo "Running the program $(OUTPUT_BIN):"
	@echo ""
	@./$(OUTPUT_BIN)

clean:
	rm -f $(BIN)/*.bin $(OBJ)/*.o -r $(HTML)

doc:
	doxygen $(DOXYFILE)

print_doc:
	$(BROWSER) $(HTML)/index.html