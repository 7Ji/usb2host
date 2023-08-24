BINARY = usb2host
CFLAGS ?= -Wall -Wextra -O3
PREFIX ?= /usr
NODE ?= fc000000.usb

${BINARY}: ${BINARY}.c
	${CC} -o $@ ${CFLAGS} $^

clean:
	rm -f ${BINARY}

TARGET_BIN = ${PREFIX}/usr/bin/${BINARY}
NAME_RULES = 83-typec2host.rules
INC_RULES = ${NAME_RULES}.in
TARGET_RULES = ${PREFIX}/usr/lib/udev/rules.d/${NAME_RULES}
NAME_SERVICE = usb2host.service
INC_SERVICE = ${NAME_SERVICE}.in
TARGET_SERVICE_PARENT = ${PREFIX}/usr/lib/systemd/system
TARGET_SERVICE = ${TARGET_SERVICE_PARENT}/${NAME_SERVICE}
TARGET_SERVICE_WANTS = ${TARGET_SERVICE_PARENT}/multi-user.target.wants

install:
	install -D --mode 755 ${BINARY} ${TARGET_BIN}

	sed "s_%BINARY%_/usr/bin/${BINARY}_;\
		 s_%NODE%_${NODE}_" \
		 ${INC_RULES} | \
	install -D --mode 644 /dev/stdin ${TARGET_RULES}

	sed "s_%BINARY%_/usr/bin/${BINARY}_;\
		 s_%NODE%_${NODE}_" \
		 ${INC_SERVICE} | \
	install -D --mode 644 /dev/stdin ${TARGET_SERVICE}

	install -D --mode 755 --directory ${TARGET_SERVICE_WANTS}
	ln -sf ../${NAME_SERVICE} ${TARGET_SERVICE_WANTS}

.PHONY: clean install