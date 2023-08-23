#!/bin/sh

# Define the compose file path
COMPOSE_FILE_PATH=${HOME}/ScienceGround2021/src/server

# Remove existing repository
sudo rm -r -f ${HOME}/ScienceGround2021

# Clone the repository using native git
git clone https://github.com/AugMantua/ScienceGround2021.git ${HOME}/ScienceGround2021

# Change to the directory containing the Docker Compose file
cd ${COMPOSE_FILE_PATH}

# Build the services using native docker-compose
docker-compose build

# Stop and remove containers, networks, images, and volumes using native docker-compose
docker-compose down

# Create services but do not start using native docker-compose
docker-compose up --no-start

# Start the services using native docker-compose
docker-compose start
