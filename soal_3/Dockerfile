FROM kalilinux/kali-rolling:latest

RUN apt-get update && apt-get install -y \
    fuse3 \
    libfuse3-dev \
    pkg-config \
    gcc \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY antink.c /app/
RUN gcc -Wall antink.c -o antink -D_FILE_OFFSET_BITS=64 -lfuse3

RUN mkdir -p /antink_mount /it24_host /var/log/it24
VOLUME ["/it24_host", "/antink_mount", "/var/log/it24"]

CMD ["/app/antink", "/antink_mount", "-f"]
