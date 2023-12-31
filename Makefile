BINARY = usb2host
CFLAGS ?= -Wall -Wextra -O3
STRIP ?= strip
PREFIX ?= /usr
NODE ?= fc000000.usb
ENABLE_SERVICE ?=
HAS_RULES ?=

${BINARY}: ${BINARY}.c
	${CC} -o $@ ${CFLAGS} $^
	${STRIP} $@

clean:
	rm -f ${BINARY}

TARGET_BIN = ${PREFIX}/usr/bin/${BINARY}
ifeq (${HAS_RULES}, 1)
NAME_RULES = 83-usb2host.rules
INC_RULES = ${NAME_RULES}.in
TARGET_RULES = ${PREFIX}/usr/lib/udev/rules.d/${NAME_RULES}
endif
NAME_SERVICE = usb2host.service
INC_SERVICE = ${NAME_SERVICE}.in
TARGET_SERVICE_PARENT = ${PREFIX}/usr/lib/systemd/system
TARGET_SERVICE = ${TARGET_SERVICE_PARENT}/${NAME_SERVICE}
ifeq (${ENABLE_SERVICE}, 1)
TARGET_SERVICE_WANTS = ${TARGET_SERVICE_PARENT}/multi-user.target.wants
endif

install: ${BINARY}
	install -D --mode 755 ${BINARY} ${TARGET_BIN}

ifeq (${HAS_RULES}, 1)
	sed "s_%BINARY%_/usr/bin/${BINARY}_;\
		 s_%NODE%_${NODE}_" \
		 ${INC_RULES} | \
	install -D --mode 644 /dev/stdin ${TARGET_RULES}
endif

	sed "s_%BINARY%_/usr/bin/${BINARY}_;\
		 s_%NODE%_${NODE}_" \
		 ${INC_SERVICE} | \
	install -D --mode 644 /dev/stdin ${TARGET_SERVICE}

ifeq (${ENABLE_SERVICE}, 1)
	install -D --mode 755 --directory ${TARGET_SERVICE_WANTS}
	ln -sf ../${NAME_SERVICE} ${TARGET_SERVICE_WANTS}
endif

.PHONY: clean install