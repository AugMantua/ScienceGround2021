package main

import (
	"fmt"
	"log"

	"github.com/gin-gonic/gin"
)

func main() {
	//DB related
	dataDBinit(_DB_NAME)

	router := gin.Default()
	router.POST("/data/add", func(c *gin.Context) {
		AddRead(c.Writer, c.Request)
	})

	fmt.Println("Serving on port 4000")
	err := router.Run(":4000")
	if err != nil {
		log.Fatal(err)
	}
}
