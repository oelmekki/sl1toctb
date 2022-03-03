PROG=sl1toctb
DEPS=zlib libzip
FILES=main.c spng.c utils.c convert.c parser.c inspect.c
CFLAGS=`pkg-config --cflags ${DEPS}`
PREFIX=/usr/local
LIBS=-lm `pkg-config --libs ${DEPS}`

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

