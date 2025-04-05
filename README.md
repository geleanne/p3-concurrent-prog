# FOR MACOS / LINUX

## Compile all components (requires C++17)
g++ consumer/consumer.cpp -o consumer/consumer.out -std=c++17 -pthread
g++ producer/producer.cpp -o producer/producer.out -std=c++17
g++ web/web_server.cpp -o web/web_server.out -std=c++17

## In Terminal 1 – Run the consumer (multithreaded upload handler)
./consumer/consumer.out

## In Terminal 2 – Run the producer (sends files from folders)
./producer/producer.out

## In Terminal 3 – Run the web server (video browser + upload support)
./web/web_server.out

## Access the frontend at:
http://localhost:8080

# FOR WINDOWS (MinGW)

## Compile all components
g++ consumer/consumer.cpp -o consumer/consumer.exe -std=c++17 -lws2_32 -pthread
g++ producer/producer.cpp -o producer/producer.exe -std=c++17 -lws2_32
g++ web/web_server.cpp -o web/web_server.exe -std=c++17

## In Command Prompt 1 (run the consumer/server)
consumer\consumer.exe

## In Command Prompt 2 (run the producer/client)
producer\producer.exe

## In Command Prompt 3 (run the web server)
web\web_server.exe

## Access the frontend at:
http://localhost:8080
