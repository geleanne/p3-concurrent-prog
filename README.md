# FOR MACOS / LINUX

## Compile both components (requires C++17)
g++ consumer/consumer.cpp -o consumer.out -std=c++17
g++ producer/producer.cpp -o producer.out -std=c++17

## In Terminal 1 (run the consumer/server)
./consumer.out

## In Terminal 2 (run the producer/client)
./producer.out

# FOR WINDOWS0S
##  Compile both components
g++ consumer/consumer.cpp -o consumer.exe -std=c++17 -lws2_32
g++ producer/producer.cpp -o producer.exe -std=c++17 -lws2_32

## In Command Prompt 1 (run the consumer/server)
consumer.exe

## In Command Prompt 2 (run the producer/client)
producer.exe
