package main

import (
	"database/sql"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"strings"

	"github.com/golang/gddo/httputil/header"
	_ "github.com/mattn/go-sqlite3"
)

func jsonDecodeToStruct(p interface{}, w http.ResponseWriter, r *http.Request) (interface{}, error) {

	r.Body = http.MaxBytesReader(w, r.Body, 1048576)
	dec := json.NewDecoder(r.Body)
	dec.DisallowUnknownFields()
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
		return nil, err
	}

	err = dec.Decode(&struct{}{})
	if err != io.EOF {
		msg := "Request body must only contain a single JSON object"
		http.Error(w, msg, http.StatusBadRequest)
		return nil, err
	}

	return p, nil
}

type measures_request_typ struct {
	TerrariumID string
	From        string
	To          string
	SensorID    string //If not set -> all
}

func AddMeasure(db *sql.DB) http.HandlerFunc {

	return func(w http.ResponseWriter, r *http.Request) {
		if r.Header.Get("Content-Type") != "" {
			value, _ := header.ParseValueAndParams(r.Header, "Content-Type")
			if value != "application/json" {
				msg := "Content-Type header is not application/json"
				http.Error(w, msg, http.StatusUnsupportedMediaType)
				return
			}
		}
		var p measures_data
		/*Decode json content*/
		jsonDecodeToStruct(&p, w, r)
		/*Data decoded*/
		fmt.Fprintf(w, "Data: %+v", p)
		/*Data insert into DB*/
		for index := range p.Data {
			t_data := p.Data[index]
			insertMeasure(db, t_data)
		}
	}
}

func RequestMeasures(db *sql.DB) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Header.Get("Content-Type") != "" {
			value, _ := header.ParseValueAndParams(r.Header, "Content-Type")
			if value != "application/json" {
				msg := "Content-Type header is not application/json"
				http.Error(w, msg, http.StatusUnsupportedMediaType)
				return
			}
		}
		var request measures_request_typ
		/*Decode json content*/
		jsonDecodeToStruct(&request, w, r)
		/*Extract data and send back*/
		measures := getMeasures(db, request)
		/*Encode to json*/
		message, err := json.Marshal(measures)
		if err != nil {
			http.Error(w, "", http.StatusInternalServerError)
			log.Fatal(err.Error())
		}
		w.Write(message)
	}
}

func RequestTerrariumsList(db *sql.DB) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		/*Extract data and send back*/
		t_terrariums := getTerrariums(db)
		/*Encode to json*/
		message, err := json.Marshal(t_terrariums)
		if err != nil {
			http.Error(w, "", http.StatusInternalServerError)
			log.Fatal(err.Error())
		}
		w.Write(message)
	}
}
