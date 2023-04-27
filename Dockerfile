FROM rodrigodornelles/sampgdk:latest AS compiler

ADD . /samp-svr

WORKDIR /samp-svr

RUN pawncc gamemodes/blank.pwn && \
    cmake . && \
    make

FROM pyrax/samp:0.3.7r2-1

ENV SAMP_GAMEMODE0="blank 1"
ENV SAMP_PLUGINS="main.so"

COPY --from=compiler /usr/local/lib/libsampgdk.so.4 /lib/libsampgdk.so.4
COPY --from=compiler /samp-svr/blank.amx /samp-svr/gamemodes/blank.amx
COPY --from=compiler /samp-svr/main.so /samp-svr/plugins/main.so
