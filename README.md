# Step 1: Build Docker Images
docker build -t consumer-app -f consumer/Dockerfile .
docker build -t producer-app -f producer/Dockerfile .
docker build -t web-server-app -f web/Dockerfile .

# Step 2: Create Docker Network (if not already exists)
docker network create media-net

# Step 3: Run Each Component in a Separate Terminal

# --- Terminal 1: Run the Consumer (receiver)
docker run -it --rm --name consumer --network media-net consumer-app

# --- Terminal 2: Run the Producer (sender)
docker run -it --rm --name producer \
  --network media-net \
  -v "$PWD/shared:/app/shared" \
  producer-app

# --- Terminal 3: Run the Web Server (frontend viewer)
docker run -it --rm --name web-server \
  -p 8080:8080 \
  --network media-net \
  web-server-app

# Step 4: Access the Frontend in Browser
# Open http://localhost:8080 in your browser
