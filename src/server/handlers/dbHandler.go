package handlers

import (
	"context"
	"errors"
	"fmt"
	"os"
	"time"

	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
)

type SensorData struct {
	ID            primitive.ObjectID `bson:"_id,omitempty"`
	Name          string             `bson:"name"`
	TypeOfMeasure string             `bson:"typeOfMeasure,omitempty"`
	Extra_data    string             `bson:"extraData,omitempty"`
}

type TerrariumData struct {
	ID              primitive.ObjectID    `bson:"_id,omitempty"`
	TypeOfTerrarium string                `bson:"typeOfTerrarium,omitempty"`
	TerrariumAlias  string                `bson:"terrariumAlias,omitempty"`
	Sensors         []SensorData          `bson:"sensorsIds,omitempty"`
	Status          string                `bson:"status,omitempty"`
	Sessions        []sessionData         `bson:"sessions,omitempty"`
	MACAddres       string                `bson:"macAddress,omitempty"`
	MagicKey        string                `bson:"magicKey,omitempty"`
	AuthState       bool                  `bson:"authState"`
	LastUpdate      []Single_measure_data `bson:"lastUpdate,omitempty"`
	LastSession     primitive.ObjectID    `bson:"lastSession,omitempty"`
	UpdateOn        string                `bson:"updateOn,omitempty"`
	LastSync        string                `bson:"lastSync,omitempty"`
}

type TerrariumGet struct {
	ID              primitive.ObjectID `bson:"_id,omitempty"`
	TypeOfTerrarium string             `bson:"typeOfTerrarium,omitempty"`
	TerrariumAlias  string             `bson:"terrariumAlias,omitempty"`
	Sensors         []SensorData       `bson:"sensorsIds,omitempty"`
	Status          string             `bson:"status,omitempty"`
	Sessions        []sessionData      `bson:"sessions,omitempty"`
	AuthState       bool               `bson:"authState"`
	LastSync        string             `bson:"lastSync,omitempty"`
}

type sessionData struct {
	SessionKey     primitive.ObjectID `bson:"sessionKey"`
	TimestampStart string             `bson:"timestampStart,omitempty"`
	TimestampEnd   string             `bson:"timestampEnd,omitempty"`
	IsAlive        bool               `bson:"isAlive"`
}

type Single_measure_data struct {
	ID          primitive.ObjectID `bson:"_id,omitempty"`
	SensorID    primitive.ObjectID `bson:"sensorId,omitempty"`
	TerrariumID primitive.ObjectID `bson:"terrariumId,omitempty"`
	Value       string             `bson:"value,omitempty"`
	Timestamp   string             `bson:"timestamp,omitempty"`
	SessionKey  primitive.ObjectID `bson:"sessionKey,omitempty"`
}

type Single_measure_data_dto struct {
	SensorID   primitive.ObjectID `bson:"sensorId,omitempty"`
	Value      string             `bson:"value,omitempty"`
	Timestamp  string             `bson:"timestamp,omitempty"`
	SessionKey primitive.ObjectID `bson:"sessionKey,omitempty"`
}

type Push_measure_request_typ struct {
	TerrariumID string `bson:"terrariumId"`
	SensorID    string `bson:"sensorId,omitempty"`
	Value       string `bson:"value,omitempty"`
	Timestamp   string `bson:"timestamp,omitempty"`
	SessionKey  string `bson:"sessionKey,omitempty"`
}

type TerrariumCredentials struct {
	MACAddres       string       `bson:"macAddress,omitempty"`
	MagicKey        string       `bson:"magicKey,omitempty"`
	TypeOfTerrarium string       `bson:"typeOfTerrarium,omitempty"`
	TerrariumAlias  string       `bson:"terrariumAlias,omitempty"`
	Sensors         []SensorData `bson:"sensorsIds,omitempty"`
}

type Measures_data struct {
	Data []Push_measure_request_typ
}

var _TERRARIUMS_COLLECTION = "terrariums"
var _MEASURES_COLLECTION = "measures"

func insertMeasures(db *mongo.Database, ctx context.Context, measures []Push_measure_request_typ) ([]Single_measure_data, error) {

	var terrarium TerrariumData
	var updateMeasures []Single_measure_data

	id, err := primitive.ObjectIDFromHex(measures[0].TerrariumID)

	for _, measure := range measures {
		var singleMeasure Single_measure_data

		if err != nil {
			return nil, errors.New("can't cast request terrariumID to objectID")
		}

		err = db.Collection(_TERRARIUMS_COLLECTION).FindOne(ctx, bson.M{"_id": id, "authState": true}).Decode(&terrarium)

		if err != nil {
			return nil, errors.New("can't find requested terrarium")
		}

		var presence bool
		presence = false
		for _, sensor := range terrarium.Sensors {
			if sensor.ID.Hex() == measure.SensorID {
				presence = true
			}
		}
		if !presence {
			return nil, errors.New("can't find requested sensor")
		}

		if measure.SessionKey != "" {
			sessionKey, _ := primitive.ObjectIDFromHex(measure.SessionKey)
			presence = false
			for _, session := range terrarium.Sessions {
				if session.SessionKey == sessionKey {
					presence = true
				}
			}
			if !presence {
				return nil, errors.New("can't find requested session")
			}
			singleMeasure.SessionKey = sessionKey
		}

		singleMeasure.ID = primitive.NewObjectIDFromTimestamp(time.Now())
		singleMeasure.TerrariumID, _ = primitive.ObjectIDFromHex(measure.TerrariumID)
		singleMeasure.SensorID, _ = primitive.ObjectIDFromHex(measure.SensorID)
		singleMeasure.Timestamp = measure.Timestamp
		singleMeasure.Value = measure.Value

		updateMeasures = append(updateMeasures, singleMeasure)
	}

	// Insert new measures
	var update bson.M
	var tempUpdate []interface{}
	for _, t := range updateMeasures {
		tempUpdate = append(tempUpdate, t)
	}
	_, err = db.Collection(_MEASURES_COLLECTION).InsertMany(ctx, tempUpdate)
	if err != nil {
		return nil, err
	}
	// Update last updateMeasure
	update = bson.M{
		"$set": bson.M{
			"lastUpdate": updateMeasures,
			"lastSync":   time.Now().Format(time.RFC3339),
			"updatedOn":  time.Now().Format(time.RFC3339),
		}}
	_, err = db.Collection(_TERRARIUMS_COLLECTION).UpdateByID(ctx, id, update)

	return updateMeasures, err
}

/*Main dataDB init
 *Check presence or create DB
 *Check Table presence or create them
 * TODO -> check context logic and replace context.Background in case
 */
func DataDBinit(dbPath string) (*mongo.Database, context.Context) {
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
func getMeasures(db *mongo.Database, request measures_request_typ, ctx context.Context) ([]Single_measure_data_dto, error) {
	var result []Single_measure_data_dto
	var filter = bson.M{}

	if request.SensorID != "" {
		id, e := primitive.ObjectIDFromHex(request.SensorID)
		if e != nil {
			return result, errors.New("validation error, sensorID")
		}
		filter["sensorId"] = id
	}
	if request.From != "" && request.To != "" {
		filter["timestamp"] = bson.M{"$gt": request.From, "$lt": request.To}
	}

	if request.TerrariumID != "" {
		id, e := primitive.ObjectIDFromHex(request.TerrariumID)
		if e != nil {
			return result, errors.New("validation error, terraiumID")
		}
		filter["terrariumId"] = id
	} else {
		return result, errors.New("validation error, terraiumID is needed")
	}

	if request.SessionKey != "" {
		idSession, _ := primitive.ObjectIDFromHex(request.SessionKey)
		filter["sessionKey"] = idSession
	}

	measuresCollection := db.Collection(_MEASURES_COLLECTION)
	var options options.FindOptions
	options.Projection = bson.M{"terrariumId": 0, "_id": 0}
	cursor, err := measuresCollection.Find(ctx, filter, &options)
	if err != nil {
		panic(err)
	}
	if err = cursor.All(ctx, &result); err != nil {
		panic(err)
	}

	return result, nil
}

func CreateSession(db *mongo.Database, ctx context.Context, terrariumID string) (string, error) {
	var session sessionData

	session.SessionKey = primitive.NewObjectIDFromTimestamp(time.Now())
	session.IsAlive = true
	var tempTerrarium TerrariumData
	var t = time.Now()
	session.TimestampStart = fmt.Sprintf("%d-%02d-%02dT%02d:%02d:%02d", t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second())
	// Get terrarium by ID
	id, err := primitive.ObjectIDFromHex(terrariumID)
	if err != nil {
		return "", errors.New("can't cast request terrariumID to objectID")
	}

	err = db.Collection(_TERRARIUMS_COLLECTION).FindOne(ctx, bson.M{"_id": id}).Decode(&tempTerrarium)
	if err != nil {
		return "", errors.New("can't get terrarium")
	}

	if tempTerrarium.LastSession != primitive.NilObjectID {
		stopSession(db, ctx, tempTerrarium.LastSession.Hex(), terrariumID)
	}

	update := bson.M{"$push": bson.M{"sessions": session}, "$set": bson.M{"lastSession": session.SessionKey}}
	_, err = db.Collection(_TERRARIUMS_COLLECTION).UpdateByID(ctx, id, update)
	if err != nil {
		return "", err
	}

	return session.SessionKey.Hex(), nil
}

func stopSession(db *mongo.Database, ctx context.Context, SessionKey string, terrariumID string) error {
	var err error

	terrariumId, _ := primitive.ObjectIDFromHex(terrariumID)
	sessionId, _ := primitive.ObjectIDFromHex(SessionKey)

	if terrariumId == primitive.NilObjectID ||
		sessionId == primitive.NilObjectID {
		return errors.New("terrarium - sessionKey mismatch")
	}

	var query = bson.M{}
	query["_id"] = terrariumId
	query["sessions.sessionKey"] = sessionId

	update := bson.M{
		"$set": bson.M{
			"sessions.$.isAlive":      false,
			"sessions.$.timestampEnd": time.Now().Format(time.RFC3339),
		},
	}

	_, err = db.Collection(_TERRARIUMS_COLLECTION).UpdateMany(ctx, query, update)

	return err
}

func getTerrariums(db *mongo.Database, ctx context.Context) []TerrariumGet {

	var terrariums []TerrariumGet

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
func tryTerrariumLogin(db *mongo.Database, ctx context.Context, request TerrariumCredentials) (TerrariumData, error) {
	var filter = bson.M{}
	var terrarium TerrariumData

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

// FIXME: avoid saving unauth terrariums if already present
func saveUnauthAttempt(db *mongo.Database, ctx context.Context, request TerrariumCredentials) error {
	unauthTerrariums := db.Collection(_TERRARIUMS_COLLECTION)

	for index, _ := range request.Sensors {
		request.Sensors[index].ID = primitive.NewObjectID()
	}

	terrarium := TerrariumData{
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
