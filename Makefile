PROG=sl1toctb
CFLAGS=
PREFIX=/usr/local
FILES=main.c utils.c
LIBS=

all: ${PROG}

${PROG}: ${FILES}
	gcc ${KIK_PROD_CFLAGS} ${CFLAGS} ${FILES} -o ${PROG} ${LIBS}

dev:
	ctags --kinds-C=+p ${FILES} *.h `project_headers`
	gcc ${KIK_DEV_CFLAGS} ${CFLAGS} ${FILES} -o ${PROG}-dev ${LIBS}

install: ${PROG}
	install -D ${PROG} ${PREFIX}/bin/${PROG}

clean:
	rm ${PROG} || true
	rm ${PROG}-dev || true

analyze:
	scan-build clang ${KIK_PROD_CFLAGS} ${FILES} -o /dev/null ${LIBS}

