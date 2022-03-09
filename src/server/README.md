Added Dockerfile
Removed sqlite db in order to add a MongoDB cluster.
Atm BE connects to MongoDB Atlas in a free tier cluster

Docker uses an alpine-go image in order to build the app and then deploys it to another conatiner based on alpine <10MB image.
This way the image is less than 20MB.

In order to build : docker build --tag science-ground-server .

In order to build with docker-compose in docker run (needed for rancherOS):  ```docker  run  -v `pwd`:`pwd` -v /var/run/docker.sock:/var/run/docker.sock -w `pwd` docker/compose build ```
