package main

import (
	"fmt"
	"log"
	"net/http"
)

func main() {
	//DB related
	SqliteDB := dataDBinit(_DB_NAME)
	//Closes db at the end of main
	defer SqliteDB.Close()
	//
	mux := http.NewServeMux()
	mux.HandleFunc("/data/add", AddMeasure(SqliteDB))
	fmt.Println("Serving on port 4000")
	err := http.ListenAndServe(":4000", mux)
	log.Fatal(err)
}
