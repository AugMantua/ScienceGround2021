package main

import (
	"context"
	"os"

	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
)

type sensorData struct {
	TypeOfMeasure string `bson:"typeOfMeasure,omitempty"`
	Extra_data    string `bson:"extraData,omitempty"`
}

type terrariumData struct {
	ID              primitive.ObjectID `bson:"_id,omitempty"`
	TypeOfTerrarium string             `bson:"typeOfTerrarium,omitempty"`
	TerrariumAlias  string             `bson:"terrariumAlias,omitempty"`
	Sensors         sensorData         `bson:"sensorsIds,omitempty"`
	Status          string             `bson:"status,omitempty"`
	Sessions        primitive.ObjectID `bson:"sessions,omitempty"`
}

type terrariumsSession struct {
	ID             primitive.ObjectID `bson:"_id,omitempty"`
	TerrariumID    primitive.ObjectID `bson:"terrariumId,omitempty"`
	SessionKey     string             `bson:"sessionKey,omitempty"`
	TimestampStart string             `bson:"timestampStart,omitempty"`
	TimestampEnd   string             `bson:"timestampEnd,omitempty"`
}

type single_measure_data struct {
	ID          primitive.ObjectID `bson:"_id,omitempty"`
	TerrariumID primitive.ObjectID `bson:"terrariumId,omitempty"`
	SensorID    primitive.ObjectID `bson:"sensorId,omitempty"`
	Value       string             `bson:"value,omitempty"`
	Timestamp   string             `bson:"timestamp,omitempty"`
	SessionKey  string             `bson:"sessionKey,omitempty"`
}

type measures_data struct {
	Data []single_measure_data
}

func insertMeasure(db *mongo.Database, measure single_measure_data) {
	_, err := db.Collection("measures").InsertOne(context.TODO(), measure)
	if err != nil {
		panic(err)
	}
}

/*Main dataDB init
 *Check presence or create DB
 *Check Table presence or create them
 * TODO -> check context logic and replace context.Background in case
 */
func dataDBinit(dbPath string) (*mongo.Database, context.Context) {
	//ctx, _ := context.WithTimeout(context.Background(), 10*time.Second)
	ctx := context.Background()
	mongoDBClient, err := mongo.Connect(ctx, options.Client().ApplyURI(os.Getenv("ATLAS_URI")))
	if err != nil {
		panic(err)
	}
	database := mongoDBClient.Database(os.Getenv("MONGODB_NAME"))
	return database, ctx
}

/*Extract measures*/
func getMeasures(db *mongo.Database, request measures_request_typ, ctx context.Context) []single_measure_data {
	var measures []single_measure_data
	var filter = bson.M{}

	if request.SensorID != "" {
		filter["sensorId"] = request.SensorID
	}
	if request.From != "" && request.To != "" {
		filter["timestamp"] = bson.M{"$gt": request.From, "$lt": request.To}
	}

	if request.TerrariumID != "" {
		filter["terrariumId"] = request.TerrariumID
	}

	measuresCollection := db.Collection("measures")
	cursor, err := measuresCollection.Find(ctx, filter)
	if err != nil {
		panic(err)
	}
	if err = cursor.All(ctx, &measures); err != nil {
		panic(err)
	}
	return measures
}

func createSessionRow(db *mongo.Database, SessionKey string, terrariumID string, timestampStart string) string {
	/*
		log.Println("Inserting session record")
		createSessionSQL := `INSERT INTO terrariumsLiveSession(terrariumID, SessionKey, timestampStart) VALUES (?,?, ?)`
		statement, err := db.Prepare(createSessionSQL) // Prepare statement.
		// This is good to avoid SQL injections
		if err != nil {
			log.Fatalln(err.Error())
		}
		_, err = statement.Exec(terrariumID, SessionKey, timestampStart)
		if err != nil {
			log.Fatalln(err.Error())
			return "err"
		}
	*/
	return SessionKey
}

func stopSession(db *mongo.Database, SessionKey string, terrariumID string, timestampEnd string) bool {
	/*log.Println("Inserting session record")
	endSessionSQL := `UPDATE terrariumsLiveSession timestampEnd	= ? WHERE SessionKey = ? AND terrariumID = ? `
	statement, err := db.Prepare(endSessionSQL) // Prepare statement.
	// This is good to avoid SQL injections
	if err != nil {
		log.Fatalln(err.Error())
	}
	_, err = statement.Exec(timestampEnd, SessionKey, terrariumID)
	if err != nil {
		log.Fatalln(err.Error())
		return false
	}
	*/
	return true
}

func getTerrariums(db *mongo.Database) []terrariumData {

	var terrariums []terrariumData

	terrariumCollection := db.Collection("terrariums")
	cursor, err := terrariumCollection.Find(context.TODO(), nil)
	if err != nil {
		panic(err)
	}
	if err = cursor.All(context.TODO(), &terrariums); err != nil {
		panic(err)
	}
	return terrariums
}
