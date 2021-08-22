##### Stage 1 #####

### Use golang:1.15 as base image for building the application
FROM golang:1.16-alpine as builder

### Create new directly and set it as working directory
RUN apk add build-base
WORKDIR /app

### Copy Go application dependency files
COPY go.mod ./
COPY go.sum ./

### Download Go application module dependencies
RUN go mod download

### Copy actual source code for building the application
COPY src/*.go ./

### CGO has to be disabled cross platform builds
### Otherwise the application won't be able to start
ENV CGO_ENABLED=1

### Build the Go app for a linux OS
### 'scratch' and 'alpine' both are Linux distributions
RUN GOOS=linux go build -o /science-ground-server

##### Stage 2 #####

### Define the running image
FROM alpine:3.13.1

### Set working directory
WORKDIR /app

### Set a volume for data-persistance
#VOLUME persistent-volume-science-ground

### Copy built binary application from 'builder' image
COPY --from=builder /science-ground-server .
### Copy .env
COPY .env ./

EXPOSE 8080

### Run the binary application
CMD ["/app/science-ground-server"]