include config.mk

BUILD = ./build/
SRC = $(shell find . -name '*.cpp' | grep -v main.cpp)
OBJ = $(patsubst %.cpp, $(BUILD)%.o, $(SRC))

all: $(BUILD)atomos

$(BUILD)atomos: $(OBJ)
	@echo producing final executable
	@$(CC) main.cpp -o $@ $(CFLAGS) $(LDFLAGS) ${OBJ}
	@echo Made final executable: $(BUILD)atomos
	@echo

$(BUILD)%.o: %.cpp
	@mkdir -p `dirname $(BUILD)$<`
	@echo compiling $<
	@$(CC) -g -c $(CFLAGS) $< -o $@

clean:
	@echo cleaning
	$(RM) -rf $(BUILD)
	@echo

run: $(BUILD)atomos
	@./$(BUILD)atomos


