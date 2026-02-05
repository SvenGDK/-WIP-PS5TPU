ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

ELF := ps5-trophy-unlocker.elf

CFLAGS := -Wall -Werror -g -O2 -Iinclude -lSceNpUniversalDataSystem -lSceSysmodule

all: $(ELF)

$(ELF): source/main.c
	$(CC) $(CFLAGS) -o $@ $^
	strip $@

clean:
	rm -f $(ELF)
