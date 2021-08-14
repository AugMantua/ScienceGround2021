package main

import (
	"fmt"
	"log"
	"net/http"
	"os"

	"github.com/joho/godotenv"
	"github.com/rs/cors"
)

func main() {

	// load .env file
	err := godotenv.Load(".env")

	if err != nil {
		log.Fatalf("Error loading .env file")
	}

	MongoDB, context := dataDBinit(os.Getenv("MONGODB"))
	//Closes db at the end of main
	defer MongoDB.Client().Disconnect(context)
	//
	mux := http.NewServeMux()
	mux.HandleFunc("/data/measures/add", AddMeasure(MongoDB))
	mux.HandleFunc("/data/measures/get", RequestMeasures(MongoDB, context))
	mux.HandleFunc("/data/terrariums/get", RequestTerrariumsList(MongoDB))

	mux.HandleFunc("/data/session/start", StartSession(MongoDB))
	mux.HandleFunc("/data/session/stop", StopSession(MongoDB))

	mux.HandleFunc("/status", Status())

	fmt.Println("Serving on port 8080")
	err = http.ListenAndServe(":8080", cors.AllowAll().Handler(mux))
	log.Fatal(err)
}
