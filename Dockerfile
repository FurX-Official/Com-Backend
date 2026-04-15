FROM ubuntu:22.04 AS builder

WORKDIR /build

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libpq-dev \
    libpqxx-dev \
    libssl-dev \
    libboost-all-dev \
    libspdlog-dev \
    nlohmann-json3-dev \
    libyaml-cpp-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth=1 https://github.com/trpc-group/trpc-cpp.git \
    && cd trpc-cpp \
    && mkdir build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc) \
    && make install \
    && cd /build && rm -rf trpc-cpp

COPY . .

RUN mkdir build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc)

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libpq5 \
    libssl3 \
    libspdlog1.9 \
    libyaml-cpp0.7 \
    libcurl4 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/build/furbbs_server ./bin/
COPY config/config.yaml ./config/

RUN mkdir -p /app/logs

EXPOSE 12345

CMD ["./bin/furbbs_server", "./config/config.yaml"]
