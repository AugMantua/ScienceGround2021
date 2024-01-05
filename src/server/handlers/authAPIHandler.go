package handlers

import (
	"context"
	"log"
	"net/http"

	"github.com/gin-gonic/gin"
	"go.mongodb.org/mongo-driver/mongo"
)

func AuthRequest(c *gin.Context) {
	dbConnection := c.MustGet("databaseConn").(*mongo.Database)
	ctx := c.MustGet("databaseCtx").(context.Context)
	var request TerrariumCredentials
	if err := c.ShouldBindJSON(&request); err != nil {
		c.JSON(http.StatusUnprocessableEntity, "Invalid json provided")
		return
	}
	terrarium, err := tryTerrariumLogin(dbConnection, ctx, request)
	if err != nil {
		c.JSON(http.StatusForbidden, gin.H{
			"message": "Forbidden",
		})
		// Store request in order to allow abilitation
		log.Println("Terrarium not found, adding to requests")
		err = saveUnauthAttempt(dbConnection, ctx, request)
		if err != nil {
			log.Println("Error creating request")
			log.Println(err.Error())
		}
		return
	}

	var sensorsMap = make(map[string]interface{})
	for _, sensor := range terrarium.Sensors {
		sensorsMap[sensor.TypeOfMeasure] = sensor
	}

	c.JSON(200, gin.H{
		"ID":      terrarium.ID,
		"Sensors": sensorsMap,
	})
}
