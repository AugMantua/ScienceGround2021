package main

import (
	"context"
	"log"
	"net/http"
	"os"

	"github.com/gin-gonic/gin"
	"github.com/joho/godotenv"
	"go.mongodb.org/mongo-driver/mongo"
)

var router = gin.Default()

// DBApiMiddleware will add the db connection to the context
func DBApiMiddleware(db *mongo.Database, ctx context.Context) gin.HandlerFunc {
	return func(c *gin.Context) {
		c.Set("databaseConn", db)
		c.Set("databaseCtx", ctx)
		c.Next()
	}
}

func main() {

	// load .env file
	err := godotenv.Load(".env")
	if err != nil { // we're not in production
		err = godotenv.Load("../.env.dev") // checking for dev env
		if err != nil {
			log.Fatalf("Error loading .env file")
		}
	}

	mongo_connection, context := dataDBinit(os.Getenv("MONGODB"))

	router.Use(DBApiMiddleware(mongo_connection, context))
	//Closes db at the end of main
	defer mongo_connection.Client().Disconnect(context)
	//
	mux := http.NewServeMux()
	router.POST("/data/measures/add", AddMeasure)
	router.GET("/data/measures/get", RequestMeasures)
	//mux.HandleFunc("/data/terrariums/get", RequestTerrariumsList(MongoDB))

	//mux.HandleFunc("/data/session/start", StartSession(MongoDB))
	//mux.HandleFunc("/data/session/stop", StopSession(MongoDB))

	mux.HandleFunc("/status", Status())

	router.Run(":8080")

	log.Fatal(err)
}
