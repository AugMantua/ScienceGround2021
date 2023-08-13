package main

import (
	"context"
	"log"
	"os"

	"github.com/gin-gonic/gin"
	"github.com/joho/godotenv"
	cors "github.com/rs/cors/wrapper/gin"
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

func clientHubMiddleware(hub *Hub) gin.HandlerFunc {
	return func(c *gin.Context) {
		c.Set("hub", hub)
		c.Next()
	}
}

func main() {

	hub := newHub()
	go hub.run()

	router.Use(cors.AllowAll())

	// load .env file
	err := godotenv.Load("./.env")
	if err != nil { // we're not in production
		err = godotenv.Load("../.env.dev") // checking for dev env
		if err != nil {
			log.Fatalf("Error loading .env file")
		}
	} else {
		gin.SetMode(gin.ReleaseMode)
	}

	mongo_connection, context := dataDBinit(os.Getenv("MONGODB"))

	//Closes db at the end of main
	defer mongo_connection.Client().Disconnect(context)
	//
	data := router.Group("/data", DBApiMiddleware(mongo_connection, context))
	{
		measures := data.Group("/measures")
		{
			measures.POST("", clientHubMiddleware(hub), AddMeasure)
			measures.GET("", RequestMeasures)
		}
		terrariums := data.Group("/terrariums")
		{
			terrariums.GET("", RequestTerrariumsList)
			terrariums.GET("/ws", clientHubMiddleware(hub), serveWs)
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

	router.Run(":" + os.Getenv("SERVER_PORT"))

	log.Fatal(err.Error())
}
