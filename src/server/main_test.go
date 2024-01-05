package main

import (
	"bytes"
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"server/handlers"
	"server/repository"
	"server/utils"
	"testing"

	"github.com/gin-gonic/gin"
	"github.com/stretchr/testify/require"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
)

func convertToJSONReader(t *testing.T, v interface{}) *bytes.Buffer {
	jsonBytes, err := json.Marshal(v)
	if err != nil {
		t.Fatal(err)
	}

	reader := bytes.NewBuffer(jsonBytes)

	return reader
}

func createTerrariumCredentialsFixture() handlers.TerrariumCredentials {
	return handlers.TerrariumCredentials{
		MACAddres:       "00:0a:95:9d:68:16",
		MagicKey:        "magicKeyExample",
		TypeOfTerrarium: "TypeExample",
		TerrariumAlias:  "TerrariumAliasExample",
		Sensors: []handlers.SensorData{
			{
				ID:            primitive.NewObjectID(),
				Name:          "SensorNameExample",
				TypeOfMeasure: "TypeOfMeasureExample",
				Extra_data:    "ExtraDataExample",
			},
			{
				ID:            primitive.NewObjectID(),
				Name:          "SensorNameExample2",
				TypeOfMeasure: "TypeOfMeasureExample2",
				Extra_data:    "ExtraDataExample2",
			},
		},
	}
}

func createMeasureFixture(terrarium handlers.TerrariumData) handlers.Measures_data {
	return handlers.Measures_data{
		Data: []handlers.Push_measure_request_typ{
			{
				TerrariumID: terrarium.ID.Hex(),
				SensorID:    terrarium.Sensors[0].ID.Hex(),
				Value:       "10",
			},
		},
	}
}

func TestMain(t *testing.T) {
	gin.SetMode(gin.TestMode)
	ctx := context.Background()

	hub := handlers.NewHub()
	go hub.Run()

	mongoClient, cleanup := utils.CreateTestMongoDBClient(ctx, t)
	database := mongoClient.Database("test")

	defer cleanup()

	router := createRouter(hub, database, ctx)

	ts := httptest.NewServer(router)
	defer ts.Close()

	t.Run("Server is running", func(t *testing.T) {
		resp, err := http.Get(ts.URL + "/status")

		require.Equal(t, err, nil, "No error is expected")
		require.Equal(t, 200, resp.StatusCode, "OK response is expected")
	})

	terrariumCredentialsFixture := createTerrariumCredentialsFixture()
	t.Run("Integration test", func(t *testing.T) {
		var testTerrarium handlers.TerrariumData
		t.Run("Auth request", func(r *testing.T) {
			// a new terrarium is trying to auth, so it should be added to the requests collection
			// a forbidden response is expected
			resp, err := http.Post(
				ts.URL+"/devices/auth",
				"application/json",
				convertToJSONReader(t, terrariumCredentialsFixture),
			)

			require.Equal(t, err, nil, "No error is expected")
			require.Equal(t, 403, resp.StatusCode, "Expected forbidden status code")

			// factory a repository to handle the requests collection
			terrariumRepository := repository.NewMongoDBRepository[handlers.TerrariumData](database.Collection("terrariums"))
			terrarium, err := terrariumRepository.Find(ctx, bson.M{"macAddress": terrariumCredentialsFixture.MACAddres})
			if err != nil {
				t.Fatal(err)
			}

			// set the auth state to true, this is to simulate the terrarium being accepted (atm this operation is done manually)
			testTerrarium = terrarium[0]
			testTerrarium.AuthState = true
			terrariumRepository.UpdateOne(ctx, bson.M{"_id": testTerrarium.ID}, testTerrarium)

			// try to auth again, now it should be successful
			resp, err = http.Post(
				ts.URL+"/devices/auth",
				"application/json",
				convertToJSONReader(t, terrariumCredentialsFixture),
			)

			require.Equal(t, err, nil, "No error is expected")
			require.Equal(t, 200, resp.StatusCode, "Expected OK status code")
		})

		var sessionKey string
		t.Run("Start session", func(r *testing.T) {
			resp, err := http.Post(
				ts.URL+"/data/terrariums/sessions/start",
				"application/json",
				convertToJSONReader(t, handlers.StartSessionRequest{TerrariumID: testTerrarium.ID.Hex()}),
			)

			// get the session key from the response as a string
			var respBody handlers.SessionKeyResponse
			err = json.NewDecoder(resp.Body).Decode(&respBody)
			if err != nil {
				t.Fatal(err)
			}
			sessionKey = respBody.SessionKey

			require.Equal(t, err, nil, "No error is expected")
			require.Equal(t, 200, resp.StatusCode, "Expected OK status code")
			require.NotEmpty(t, sessionKey, "Expected a session key")
		})

		measure := createMeasureFixture(testTerrarium)
		t.Run("Send a measure", func(r *testing.T) {
			resp, err := http.Post(
				ts.URL+"/data/measures",
				"application/json",
				convertToJSONReader(
					t,
					measure,
				),
			)

			require.Equal(t, err, nil, "No error is expected")
			require.Equal(t, 200, resp.StatusCode, "Expected OK status code")
		})

		t.Run("Get measures", func(r *testing.T) {
			resp, err := http.Get(
				ts.URL + "/data/measures?TerrariumID=" + testTerrarium.ID.Hex(),
			)

			require.Equal(t, err, nil, "No error is expected")
			require.Equal(t, 200, resp.StatusCode, "Expected OK status code")

			var measures handlers.Measures_data
			err = json.NewDecoder(resp.Body).Decode(&measures)
			if err != nil {
				t.Fatal(err)
			}

			require.Equal(t, measure.Data[0].Value, measures.Data[0].Value, "Expected the same value")
			require.Equal(t, measure.Data[0].SensorID, measures.Data[0].SensorID, "Expected the same sensor ID")
		})

		t.Run("Shutdown session", func(r *testing.T) {
			resp, err := http.Post(
				ts.URL+"/data/terrariums/sessions/stop",
				"application/json",
				convertToJSONReader(t, handlers.StopSessionRequest{
					TerrariumID: testTerrarium.ID.Hex(),
					SessionKey:  sessionKey,
				}),
			)

			require.Equal(t, err, nil, "No error is expected")
			require.Equal(t, 200, resp.StatusCode, "Expected OK status code")
		})

		// TODO: test the websocket connection
	})
}
