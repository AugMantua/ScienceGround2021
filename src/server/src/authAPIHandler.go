package main

import (
	"context"
	"net/http"

	"github.com/gin-gonic/gin"
	"go.mongodb.org/mongo-driver/mongo"
)

func AuthRequest(c *gin.Context) {
	dbConnection := c.MustGet("databaseConn").(*mongo.Database)
	ctx := c.MustGet("databaseCtx").(context.Context)
	var request terrariumCredentials
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
		saveUnauthAttempt(dbConnection, ctx, request)
		return
	}
	c.JSON(200, gin.H{
		"data": terrarium.ID,
	})
}
