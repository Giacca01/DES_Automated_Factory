CC = $(shell which g++)
SRC = MVA.cpp
OUT = mva

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT)

run: $(OUT)
	@echo "Using compiler: $(CC)"
	./$(OUT)

clean:
	rm -f $(OUT)

