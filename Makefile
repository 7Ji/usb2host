BINARY = usb2host
CFLAGS ?= -Wall -Wextra -O3
${BINARY}: ${BINARY}.c
	${CC} -o $@ ${CFLAGS} $^

clean:
	rm -f ${BINARY}

.PHONY: clean