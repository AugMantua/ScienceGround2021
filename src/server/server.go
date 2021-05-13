package main

import (
	"fmt"
	"log"
	"net/http"
)

func main() {
	//DB related
	dataDBinit(_DB_NAME)
	mux := http.NewServeMux()
	mux.HandleFunc("/data/add", AddRead)
	fmt.Println("Serving on port 4000")
	err := http.ListenAndServe(":4000", mux)
	log.Fatal(err)
}
