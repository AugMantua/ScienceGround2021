package main

import (
	"database/sql"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"strings"

	"github.com/golang/gddo/httputil/header"
	_ "github.com/mattn/go-sqlite3"
)

const _DB_NAME = "sqlite-scienceground.db"

var _DB_TABLES = []string{
	"measures",
	"sensors",
}

type Data struct {
	Name  string
	Value float64
}

type request_data struct {
	Data []Data
}

func InitNewDBFile() {
	os.Remove(_DB_NAME)
	file, err := os.Create(_DB_NAME) // Create SQLite file
	if err != nil {
		log.Fatal(err.Error())
	}
	file.Close()
	log.Println(_DB_NAME)
}

func CheckDBFile(dbPath string) bool {
	if _, err := os.Stat(dbPath); os.IsNotExist(err) {
		return false
	} else {
		return true
	}
}

func CheckDBTables(db *sql.DB, standardTables []string) bool {
	flag := true
	for index := range standardTables {
		_, table_check := db.Query("select * from " + standardTables[index] + ";")
		if table_check == nil {
			fmt.Println("table is there")
		} else {
			fmt.Println("table not there")
			flag = false
		}
	}
	return flag
}

/*Main dataDB init
 *Check presence or create DB
 *Check Table presence or create them
 */
func dataDBinit(dbPath string) {
	if !CheckDBFile(dbPath) {
		InitNewDBFile()
	}
	sqliteDatabase, _ := sql.Open("sqlite3", "./"+dbPath) //Open sqliteDB
	defer sqliteDatabase.Close()                          // Defer Closing the database
	//Check db tables existence
	if !CheckDBTables(sqliteDatabase, _DB_TABLES) {
		//TODO -> create tables
	}
}

func AddRead(w http.ResponseWriter, r *http.Request) {

	if r.Header.Get("Content-Type") != "" {
		value, _ := header.ParseValueAndParams(r.Header, "Content-Type")
		if value != "application/json" {
			msg := "Content-Type header is not application/json"
			http.Error(w, msg, http.StatusUnsupportedMediaType)
			return
		}
	}

	r.Body = http.MaxBytesReader(w, r.Body, 1048576)

	dec := json.NewDecoder(r.Body)
	dec.DisallowUnknownFields()

	var p request_data
	err := dec.Decode(&p)
	if err != nil {
		var syntaxError *json.SyntaxError
		var unmarshalTypeError *json.UnmarshalTypeError

		switch {

		case errors.As(err, &syntaxError):
			msg := fmt.Sprintf("Request body contains badly-formed JSON (at position %d)", syntaxError.Offset)
			http.Error(w, msg, http.StatusBadRequest)

		case errors.Is(err, io.ErrUnexpectedEOF):
			msg := fmt.Sprintf("Request body contains badly-formed JSON")
			http.Error(w, msg, http.StatusBadRequest)

		case errors.As(err, &unmarshalTypeError):
			msg := fmt.Sprintf("Request body contains an invalid value for the %q field (at position %d)", unmarshalTypeError.Field, unmarshalTypeError.Offset)
			http.Error(w, msg, http.StatusBadRequest)
		case strings.HasPrefix(err.Error(), "json: unknown field "):
			fieldName := strings.TrimPrefix(err.Error(), "json: unknown field ")
			msg := fmt.Sprintf("Request body contains unknown field %s", fieldName)
			http.Error(w, msg, http.StatusBadRequest)

		case errors.Is(err, io.EOF):
			msg := "Request body must not be empty"
			http.Error(w, msg, http.StatusBadRequest)

		case err.Error() == "http: request body too large":
			msg := "Request body must not be larger than 1MB"
			http.Error(w, msg, http.StatusRequestEntityTooLarge)

		default:
			log.Println(err.Error())
			http.Error(w, http.StatusText(http.StatusInternalServerError), http.StatusInternalServerError)
		}
		return
	}

	err = dec.Decode(&struct{}{})
	if err != io.EOF {
		msg := "Request body must only contain a single JSON object"
		http.Error(w, msg, http.StatusBadRequest)
		return
	}

	/*Data decoded*/

	fmt.Fprintf(w, "Data: %+v", p)
}
