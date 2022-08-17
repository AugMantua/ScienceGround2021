#!/bin/sh
COMPOSE_FILE_PATH = ${HOME}/ScienceGround2021/src/server/ 
sudo rm -r -f ${HOME}/ScienceGround2021 
docker run -ti --rm -v ${HOME}:/rancher -v $(pwd):/git alpine/git clone https://github.com/AugMantua/ScienceGround2021.git
docker  run  -v `pwd`:`pwd` -v /var/run/docker.sock:/var/run/docker.sock -w `pwd` docker/compose:alpine-1.27.1 ${COMPOSE_FILE_PATH} build
docker  run  -v `pwd`:`pwd` -v /var/run/docker.sock:/var/run/docker.sock -w `pwd` docker/compose:alpine-1.27.1 ${COMPOSE_FILE_PATH} down
docker  run  -v `pwd`:`pwd` -v /var/run/docker.sock:/var/run/docker.sock -w `pwd` docker/compose:alpine-1.27.1 ${COMPOSE_FILE_PATH} up