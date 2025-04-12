FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    g++ \
    make && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY .. .

RUN g++ -std=c++17 -pthread -o consumer.out consumer/consumer.cpp
CMD ["./consumer.out"]
