ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

TARGET := ps5_trophy_unlocker.elf

SOURCES := main.c
OBJECTS := $(SOURCES:.c=.o)

CFLAGS := -Wall -O2

LDFLAGS := -lkernel \
           -lSceUserService \
           -lSceSysmodule \
           -lSceNpUniversalDataSystem

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

test:
	$(PS5_PAYLOAD_SDK)/host/send-payload.sh $(TARGET) $(PS5_HOST) $(PS5_PORT)

.PHONY: all clean test