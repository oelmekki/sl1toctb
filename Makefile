PROG=sl1toctb
DEPS=zlib libzip
FILES=main.c spng.c utils.c convert.c parser.c inspect.c
DEV_CFLAGS=-std=c18 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter -O0 -g3 -ggdb3 -fsanitize=undefined
PROD_CFLAGS=-std=c18 -D_POSIX_C_SOURCE=200809L -O2 -pipe -march=native
CFLAGS=`pkg-config --cflags ${DEPS}`
PREFIX=/usr/local
LIBS=-lm `pkg-config --libs ${DEPS}`

all: ${PROG}

${PROG}: ${FILES}
	gcc ${PROD_CFLAGS} ${CFLAGS} ${FILES} -o ${PROG} ${LIBS}

dev:
	ctags --kinds-C=+p ${FILES} *.h `project_headers`
	gcc ${DEV_CFLAGS} ${CFLAGS} ${FILES} -o ${PROG}-dev ${LIBS}

install: ${PROG}
	install -D ${PROG} ${PREFIX}/bin/${PROG}

clean:
	rm ${PROG} || true
	rm ${PROG}-dev || true

analyze:
	scan-build clang ${PROD_CFLAGS} ${FILES} -o /dev/null ${LIBS}

