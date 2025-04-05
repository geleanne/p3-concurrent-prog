# FOR MACOS / LINUX
## Compile all components (requires C++17)
g++ consumer/consumer.cpp -o consumer/consumer.out -std=c++17
g++ producer/producer.cpp -o producer/producer.out -std=c++17
g++ web/web_server.cpp -o web/web_server.out -std=c++17

## In Terminal 1 (run the consumer/server)
./consumer/consumer.out

## In Terminal 2 (run the producer/client)
./producer/producer.out

## In Terminal 3 (run the web server)
./web/web_server.out


# FOR WINDOWS (pls check if this works i just chatgpt this)
## Compile all components
g++ consumer/consumer.cpp -o consumer/consumer.exe -std=c++17 -lws2_32
g++ producer/producer.cpp -o producer/producer.exe -std=c++17 -lws2_32
g++ web/web_server.cpp -o web/web_server.exe -std=c++17

## In Command Prompt 1 (run the consumer/server)
consumer\consumer.exe

## In Command Prompt 2 (run the producer/client)
producer\producer.exe

## In Command Prompt 3 (run the web server)
web\web_server.exe
