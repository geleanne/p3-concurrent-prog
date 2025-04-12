FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    zip \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY producer/producer.cpp .        
COPY ../shared/ shared/     

RUN g++ -std=c++17 -pthread -o producer.out producer.cpp

CMD ["./producer.out"]
