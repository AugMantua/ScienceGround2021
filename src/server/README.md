Added Dockerfile
Atm the container needs a volume named : /PERSISTENT-VOLUME-SCIENCE-GROUND mounted of the root of the container
This volume contains the sqlite db, this way db can be shared between istances and allows backup.

/PERSISTENT-VOLUME-SCIENCE-GROUND/sqlite-scienceground.db is the path of the DB.

Docker uses an alpine-go image in order to build the app and then deploys it to another conatiner based on alpine <10MB image.
This way the image is less than 20MB.