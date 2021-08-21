package main

import (
	"context"
	"errors"
	"os"
	"time"

	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
)

type sensorData struct {
	ID            primitive.ObjectID `bson:"_id,omitempty"`
	Name          string             `bson:"name"`
	TypeOfMeasure string             `bson:"typeOfMeasure,omitempty"`
	Extra_data    string             `bson:"extraData,omitempty"`
}

type terrariumData struct {
	ID              primitive.ObjectID    `bson:"_id,omitempty"`
	TypeOfTerrarium string                `bson:"typeOfTerrarium,omitempty"`
	TerrariumAlias  string                `bson:"terrariumAlias,omitempty"`
	Sensors         []sensorData          `bson:"sensorsIds,omitempty"`
	Status          string                `bson:"status,omitempty"`
	Sessions        primitive.ObjectID    `bson:"sessions,omitempty"`
	MACAddres       string                `bson:"macAddress,omitempty"`
	MagicKey        string                `bson:"magicKey,omitempty"`
	AuthState       bool                  `bson:"authState"`
	Measures        []single_measure_data `bson:"measures"`
}

type terrariumGet struct {
	ID              primitive.ObjectID `bson:"_id,omitempty"`
	TypeOfTerrarium string             `bson:"typeOfTerrarium,omitempty"`
	TerrariumAlias  string             `bson:"terrariumAlias,omitempty"`
	Sensors         []sensorData       `bson:"sensorsIds,omitempty"`
	Status          string             `bson:"status,omitempty"`
	Sessions        primitive.ObjectID `bson:"sessions,omitempty"`
	AuthState       bool               `bson:"authState"`
}

type terrariumsSession struct {
	ID             primitive.ObjectID `bson:"_id,omitempty"`
	TerrariumID    primitive.ObjectID `bson:"terrariumId,omitempty"`
	SessionKey     string             `bson:"sessionKey,omitempty"`
	TimestampStart string             `bson:"timestampStart,omitempty"`
	TimestampEnd   string             `bson:"timestampEnd,omitempty"`
}

type single_measure_data struct {
	ID         primitive.ObjectID `bson:"_id,omitempty"`
	SensorName string             `bson:"sensorId,omitempty"`
	Value      string             `bson:"value,omitempty"`
	Timestamp  string             `bson:"timestamp,omitempty"`
	SessionKey string             `bson:"sessionKey,omitempty"`
}

type push_measure_request_typ struct {
	TerrariumID string `bson:"terrariumId"`
	SensorName  string `bson:"sensorId,omitempty"`
	Value       string `bson:"value,omitempty"`
	Timestamp   string `bson:"timestamp,omitempty"`
	SessionKey  string `bson:"sessionKey,omitempty"`
}

type terrariumCredentials struct {
	MACAddres       string       `bson:"macAddress,omitempty"`
	MagicKey        string       `bson:"magicKey,omitempty"`
	TypeOfTerrarium string       `bson:"typeOfTerrarium,omitempty"`
	TerrariumAlias  string       `bson:"terrariumAlias,omitempty"`
	Sensors         []sensorData `bson:"sensorsIds,omitempty"`
}

type measures_data struct {
	Data []push_measure_request_typ
}

var _TERRARIUMS_COLLECTION = "terrariums"

func insertMeasure(db *mongo.Database, ctx context.Context, measure push_measure_request_typ) error {

	var terrarium terrariumData
	var singleMeasure single_measure_data

	id, err := primitive.ObjectIDFromHex(measure.TerrariumID)

	if err != nil {
		return errors.New("can't cast request terrariumID to objectID")
	}

	err = db.Collection(_TERRARIUMS_COLLECTION).FindOne(ctx, bson.M{"_id": id, "authState": true}).Decode(&terrarium)

	if err != nil {
		return errors.New("can't find requested terrarium")
	}

	var presence bool
	presence = false
	for _, sensor := range terrarium.Sensors {
		if sensor.Name == measure.SensorName {
			presence = true
		}
	}
	if !presence {
		return errors.New("can't find requested sensor")
	}

	singleMeasure.ID = primitive.NewObjectIDFromTimestamp(time.Now())
	singleMeasure.SensorName = measure.SensorName
	singleMeasure.SessionKey = measure.SessionKey
	singleMeasure.Timestamp = measure.Timestamp
	singleMeasure.Value = measure.Value

	var update bson.M
	if len(terrarium.Measures) != 0 {
		update = bson.M{"$push": bson.M{"measures": singleMeasure}}
	} else {
		update = bson.M{"$set": bson.M{"measures": singleMeasure}}
	}

	_, err = db.Collection("terrarium").UpdateByID(ctx, terrarium.ID, update)
	if err != nil {
		return err
	}
	return nil
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

func getTerrariums(db *mongo.Database, ctx context.Context) []terrariumGet {

	var terrariums []terrariumGet

	terrariumCollection := db.Collection(_TERRARIUMS_COLLECTION)
	cursor, err := terrariumCollection.Find(ctx, bson.M{"authState": true})
	if err != nil {
		panic(err)
	}
	if err = cursor.All(ctx, &terrariums); err != nil {
		panic(err)
	}
	return terrariums
}

/*Try login*/
func tryTerrariumLogin(db *mongo.Database, ctx context.Context, request terrariumCredentials) (terrariumData, error) {
	var filter = bson.M{}
	var terrarium terrariumData

	if request.MACAddres != "" && request.MagicKey != "" {
		filter["macAddress"] = request.MACAddres
		filter["magicKey"] = request.MagicKey
		filter["authState"] = true
	} else {
		return terrarium, errors.New("auth not found")
	}

	terrariumsCollection := db.Collection(_TERRARIUMS_COLLECTION)
	element := terrariumsCollection.FindOne(ctx, filter)

	if err := element.Decode(&terrarium); err != nil {
		return terrarium, errors.New("auth not found")
	}
	return terrarium, nil
}

func saveUnauthAttempt(db *mongo.Database, ctx context.Context, request terrariumCredentials) error {
	unauthTerrariums := db.Collection(_TERRARIUMS_COLLECTION)

	for index, _ := range request.Sensors {
		request.Sensors[index].ID = primitive.NewObjectID()
	}

	terrarium := terrariumData{
		MACAddres:       request.MACAddres,
		MagicKey:        request.MagicKey,
		TerrariumAlias:  request.TerrariumAlias,
		TypeOfTerrarium: request.TypeOfTerrarium,
		AuthState:       false,
		Sensors:         request.Sensors,
	}

	_, err := unauthTerrariums.InsertOne(ctx, terrarium)
	return err
}
