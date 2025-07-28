CC = $(shell which g++)
SRC = NESssq.cpp NESssq_List_Manager.cpp random_var_gen.cpp rngs.cpp
OUT = simulator

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT)

run: $(OUT)
	@echo "Using compiler: $(CC)"
	@echo "Usage: make run PRINTOUT=<your_flag>"
	./$(OUT) $(PRINTOUT)

clean:
	rm -f $(OUT)

