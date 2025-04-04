# Compile both components
g++ consumer/consumer.cpp -o consumer.out
g++ producer/producer.cpp -o producer.out

# In Terminal 1 (run the consumer/server)
./consumer.out

# In Terminal 2 (run the producer/client)
./producer.out
