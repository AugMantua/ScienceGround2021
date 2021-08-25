package main

import (
	"context"
	"log"
	"os"

	"github.com/gin-contrib/cors"
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

	//Closes db at the end of main
	defer mongo_connection.Client().Disconnect(context)
	//
	data := router.Group("/data", DBApiMiddleware(mongo_connection, context))
	{
		measures := data.Group("/measures")
		{
			measures.POST("/add", AddMeasure)
			measures.GET("/get", RequestMeasures)
		}
		terrariums := data.Group("/terrariums")
		{
			terrariums.GET("/get", RequestTerrariumsList)
			sessions := terrariums.Group("/sessions")
			{
				sessions.POST("/start", StartSession)
				sessions.POST("/stop", StopSession)
			}
		}
	}

	devices := router.Group("/devices", DBApiMiddleware(mongo_connection, context))
	{
		devices.POST("/auth", AuthRequest)
	}

	router.GET("/status", Status)

	router.Use(cors.Default())

	router.Run(":8080")

	log.Fatal(err)
}
