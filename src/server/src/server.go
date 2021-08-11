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

	SqliteDB := dataDBinit(os.Getenv("SQLITE_DB_PATH"))
	//Closes db at the end of main
	defer SqliteDB.Close()
	//
	mux := http.NewServeMux()
	mux.HandleFunc("/data/measures/add", AddMeasure(SqliteDB))
	mux.HandleFunc("/data/measures/get", RequestMeasures(SqliteDB))
	mux.HandleFunc("/data/terrariums/get", RequestTerrariumsList(SqliteDB))

	mux.HandleFunc("/data/session/start", StartSession(SqliteDB))
	mux.HandleFunc("/data/session/stop", StopSession(SqliteDB))

	mux.HandleFunc("/status", Status())

	fmt.Println("Serving on port 8080")
	err = http.ListenAndServe(":8080", cors.AllowAll().Handler(mux))
	log.Fatal(err)
}
