package handlers

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
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
	TerrariumID    string
	From           string
	To             string
	SensorID       string //If not set -> all
	SessionKey     string
	LastUpdateOnly bool
}

type wsSubscribeRequestType struct {
	TerraiumID string
}

func AddMeasure(c *gin.Context) {
	dbConnection := c.MustGet("databaseConn").(*mongo.Database)
	ctx := c.MustGet("databaseCtx").(context.Context)
	hub := c.MustGet("hub").(*Hub)

	var measures_input Measures_data
	if err := c.ShouldBindJSON(&measures_input); err != nil {
		c.JSON(http.StatusUnprocessableEntity, "Invalid json provided")
		return
	}
	if len(measures_input.Data) == 0 {
		c.JSON(http.StatusBadRequest, "Bad request")
		return
	}

	newMeasuers, err := insertMeasures(dbConnection, ctx, measures_input.Data)
	if err != nil {
		c.JSON(http.StatusBadRequest, err.Error())
		return
	}
	// Update clients registered for terrarium
	hub.Broadcast <- newMeasuers
}

func RequestMeasures(c *gin.Context) {
	dbConnection := c.MustGet("databaseConn").(*mongo.Database)
	ctx := c.MustGet("databaseCtx").(context.Context)
	var request measures_request_typ

	if err := c.ShouldBind(&request); err != nil {
		c.JSON(http.StatusUnprocessableEntity, "Invalid query provided")
		return
	}
	measures, err := getMeasures(dbConnection, request, ctx)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"error": err.Error(),
		})
		return
	}
	c.JSON(200, gin.H{
		"data": measures,
	})

}

func Status(c *gin.Context) {
	c.JSON(http.StatusOK, gin.H{
		"Status": "OK",
	})
}

type StartSessionRequest struct {
	TerrariumID string
}

type SessionKeyResponse struct {
	SessionKey string `json:"SessionKey"`
}

func StartSession(c *gin.Context) {
	dbConnection := c.MustGet("databaseConn").(*mongo.Database)
	ctx := c.MustGet("databaseCtx").(context.Context)

	var sessionRequest StartSessionRequest
	if err := c.ShouldBindJSON(&sessionRequest); err != nil {
		c.JSON(http.StatusUnprocessableEntity, "Invalid json provided")
		return
	}

	sessionKey, err := CreateSession(dbConnection, ctx, sessionRequest.TerrariumID)
	if err != nil {
		c.JSON(http.StatusBadRequest, err)
		return
	}
	c.JSON(200, gin.H{
		"SessionKey": sessionKey,
	})
}

type StopSessionRequest struct {
	TerrariumID string
	SessionKey  string
}

func StopSession(c *gin.Context) {
	dbConnection := c.MustGet("databaseConn").(*mongo.Database)
	ctx := c.MustGet("databaseCtx").(context.Context)

	var req StopSessionRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusUnprocessableEntity, "Invalid json provided")
		return
	}

	if err := stopSession(dbConnection, ctx, req.SessionKey, req.TerrariumID); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"error": err,
		})
		return
	}

	c.JSON(200, gin.H{
		"done": true,
	})
}

func RequestTerrariumsList(c *gin.Context) {

	dbConnection := c.MustGet("databaseConn").(*mongo.Database)
	ctx := c.MustGet("databaseCtx").(context.Context)

	/*Extract data and send back*/
	t_terrariums := getTerrariums(dbConnection, ctx)

	c.JSON(200, gin.H{
		"data": t_terrariums,
	})

}
