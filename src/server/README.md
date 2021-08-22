Added Dockerfile
Removed sqlite db in order to add a MongoDB cluster.
Atm BE connects to MongoDB Atlas in a free tier cluster

Docker uses an alpine-go image in order to build the app and then deploys it to another conatiner based on alpine <10MB image.
This way the image is less than 20MB.

In order to build : docker build --tag science-ground-server .