CFLAGS += -Wall
LDFLAGS := $(LDFLAGS)

APP = test_hc-sr04
SRC = test_hc-sr04.c

all: $(APP)

$(APP): $(SRC)
	$(CC) $(LDFLAGS) $(SRC) -o $(APP)

clean:
	$(RM) $(APP) *.o
