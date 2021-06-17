package main

import (
	"fmt"
	"log"
	"net/http"

	"github.com/rs/cors"
)

func main() {

	/*CORS*/
	c := cors.New(cors.Options{
		AllowedOrigins: []string{"*"},
	})

	//DB related
	SqliteDB := dataDBinit(_DB_NAME)
	//Closes db at the end of main
	defer SqliteDB.Close()
	//
	mux := http.NewServeMux()
	mux.HandleFunc("/data/measures/add", AddMeasure(SqliteDB))
	mux.HandleFunc("/data/measures/get", RequestMeasures(SqliteDB))
	mux.HandleFunc("/data/terrariums/get", RequestTerrariumsList(SqliteDB))
	fmt.Println("Serving on port 8080")
	err := http.ListenAndServe(":8080", c.Handler(mux))
	log.Fatal(err)
}
