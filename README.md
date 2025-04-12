# P3 Networked Producer and Consumer

This project simulates a media upload system using C++ producers and consumers communicating via sockets. Videos are uploaded concurrently, saved, and previewed on a web interface. It runs entirely in Docker, showcasing multithreading, networking, and a live video frontend.

<img width="827" alt="Screenshot 2025-04-12 at 6 23 52 PM" src="https://github.com/user-attachments/assets/923b02ba-9aa4-4b4a-a1de-401051ac30b3" />

### Authors: STDISCM S12 discm buddies 
- BERNARDO, ERICA MONTEMAYOR
- ATIENZA, MARIELLE ANGELENE
- RIVERA, JOSE CARLOS IGNACIO
- TIGHE, KAITLYN PATRICIA

Included in this repository are:
1. Source code (including bonus functions for queue is full & duplication detection only)
3. [Presentation Slides](https://www.canva.com/design/DAGj1o48Lc4/JX5FxF1UG7Cqiyb_CsC2Ag/view?utm_content=DAGj1o48Lc4&utm_campaign=designshare&utm_medium=link2&utm_source=uniquelinks&utlId=hdda2d4ca40) - for further discussion
4. [P3 Demo Video](https://www.youtube.com/watch?v=ajK4hW14-zQ) - timestamps available in the description of the video 



# Build instructions (on MacOs)
## Step 1: Build Docker Images
```
docker build -t consumer-app -f consumer/Dockerfile .
docker build -t producer-app -f producer/Dockerfile .
docker build -t web-app -f web/Dockerfile .
```

## Step 2: Create Docker Network (if not already exists)
```
docker network create media-net
```

## Step 3: Run Each Component in a Separate Terminal

### - Terminal 1: Run the Consumer (receiver)
```
docker run -it --rm --name consumer \
  --network media-net \
  -v "$PWD/web/uploads:/app/web/uploads" \
  consumer-app
```

### - Terminal 2: Run the Producer (sender)
```
docker run -it --rm --name producer \
  --network media-net \
  -v "$PWD/shared:/app/shared" \
  producer-app
```


### - Terminal 3: Run the Web Server (frontend viewer)
```
docker run -it --rm --name web-server \
  --network media-net \
  -p 8080:8080 \
  -v "$PWD/web/uploads:/app/web/uploads" \
  web-app
```


## Step 4: Access the Frontend in Browser
Open http://localhost:8080 in your browser
