package main

import (
	"context"
	"crypto/rand"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"strings"
	"time"

	"github.com/golang/gddo/httputil/header"
	"go.mongodb.org/mongo-driver/mongo"
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

func AddMeasure(db *mongo.Database) http.HandlerFunc {

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

func Status() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		message := struct{ Status string }{Status: "OK"}
		res, err := json.Marshal(message)
		if err != nil {
			http.Error(w, "", http.StatusInternalServerError)
			log.Fatal(err.Error())
		}
		w.WriteHeader(http.StatusOK)
		w.Write(res)
	}
}

func StartSession(db *mongo.Database) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Header.Get("Content-Type") != "" {
			value, _ := header.ParseValueAndParams(r.Header, "Content-Type")
			if value != "application/json" {
				msg := "Content-Type header is not application/json"
				http.Error(w, msg, http.StatusUnsupportedMediaType)
				return
			}
		}
		var request terrariumsSession
		/*Decode json content*/
		jsonDecodeToStruct(&request, w, r)

		var t = time.Now()
		var timestamp = fmt.Sprintf("%d-%02d-%02dT%02d:%02d:%02d", t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second())

		const alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
		var bytes = make([]byte, 32)
		rand.Read(bytes)
		for i, b := range bytes {
			bytes[i] = alphanum[b%byte(len(alphanum))]
		}

		var newSK = string(bytes)

		/*Extract data and send back*/
		var ris = createSessionRow(db, newSK, request.TerrariumID.String(), timestamp)
		/*Encode to json*/
		message, err := json.Marshal(ris)
		if err != nil {
			http.Error(w, "", http.StatusInternalServerError)
			log.Fatal(err.Error())
		}
		w.Write(message)
	}
}

func StopSession(db *mongo.Database) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Header.Get("Content-Type") != "" {
			value, _ := header.ParseValueAndParams(r.Header, "Content-Type")
			if value != "application/json" {
				msg := "Content-Type header is not application/json"
				http.Error(w, msg, http.StatusUnsupportedMediaType)
				return
			}
		}
		var request terrariumsSession
		/*Decode json content*/
		jsonDecodeToStruct(&request, w, r)

		var t = time.Now()
		var timestamp = fmt.Sprintf("%d-%02d-%02dT%02d:%02d:%02d", t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second())

		/*Extract data and send back*/
		var ris = stopSession(db, request.SessionKey, string(request.TerrariumID.String()), timestamp)
		/*Encode to json*/
		message, err := json.Marshal(ris)
		if err != nil {
			http.Error(w, "", http.StatusInternalServerError)
			log.Fatal(err.Error())
		}
		w.Write(message)
	}
}

func RequestMeasures(db *mongo.Database, context context.Context) http.HandlerFunc {
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
		measures := getMeasures(db, request, context)
		/*Encode to json*/
		message, err := json.Marshal(measures)
		if err != nil {
			http.Error(w, "", http.StatusInternalServerError)
			log.Fatal(err.Error())
		}
		w.Write(message)
	}
}

func RequestTerrariumsList(db *mongo.Database) http.HandlerFunc {
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
