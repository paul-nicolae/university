The web service has been implemented using:
  - NodeJS for the API
  - PostgreSQL for the database
  - pgadmin for the database control interface

To create and run the 3 containers, we use two files:
Dockerfile, which builds the API container (a file where constants are defined
to run the API anywhere) and
docker-compose.yml used to run the project (a file where
the 3 containers are defined so that they can run together
in an isolated environment).

Since we want an independent organization of containers,
API and the database belong to one network (api_network), while
the database and pgadmin share another network (pgadmin_network). We
made this division so that the containers for API and pgadmin
cannot communicate.

A .dockerignore file has been created to prevent local modules and
debug logs from being copied into the Docker image and overwriting
possible modules installed within our image.

In server.js, all the web server logic is written using the Express.js framework.
Every requested route is written in the statement.

To start the project, the following command must be run:
  chmod +x deploy.sh
  docker-compose build
  docker-compose up

The pgadmin interface can be found at localhost:80/

The REST API runs at localhost:3000/

To test the project, the Tema2_script_testare.json file must be imported into Postman,
and the requests must be executed in the recommended order in the assignment statement.

Nicolae Paul-Gabriel
343C3
