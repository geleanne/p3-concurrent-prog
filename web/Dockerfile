FROM ubuntu:20.04

RUN apt-get update && apt-get install -y g++ make curl

WORKDIR /app
COPY .. .

RUN g++ -std=c++17 -pthread -o web_server.out web/web_server.cpp
CMD ["./web_server.out"]
