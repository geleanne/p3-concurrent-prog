### FOR MACOS
# Compile both components
g++ consumer/consumer.cpp -o consumer.out
g++ producer/producer.cpp -o producer.out

# In Terminal 1 (run the consumer/server)
./consumer.out

# In Terminal 2 (run the producer/client)
./producer.out

### FOR WINDOWSOS (pls check if this works i just ask chatgpt to make this)
# Compile both components
g++ consumer/consumer.cpp -o consumer.exe -lws2_32
g++ producer/producer.cpp -o producer.exe -lws2_32

# In Terminal 1 (run the consumer/server)
consumer.exe

# In Terminal 2 (run the producer/client)
producer.exe