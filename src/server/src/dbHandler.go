package main

import (
	"database/sql"
	"fmt"
	"log"
	"os"
)

const _DB_NAME = "sqlite-scienceground.db"

var _DB_TABLES = []string{
	"measures",
	"sensors",
	"terrariums",
	"terrariumsSensors",
}

type sensorData struct {
	SensorID      string
	TypeOfMeasure string
	Extra_data    string
}

type terrariumData struct {
	TerrariumID     string
	TypeOfTerrarium string
	TerrariumAlias  string
	SensorsIds      []sensorData
	Status          string
}

type terrariumsSensors struct {
	TerrariumID string
	SensorID    string
}

type terrariumsSession struct {
	TerrariumID string
	SessionKey    string
	TimestampStart   string
	TimestampEnd   string
}

type single_measure_data struct {
	TerrariumID string
	SensorID    string
	Value       string
	Timestamp   string
	SessionKey  string
}

type measures_data struct {
	Data []single_measure_data
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

func CreateDBTables(db *sql.DB) {
	/*Measures*/
	createMeasureTable := `CREATE TABLE measures (
		"idMeasure" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
		"terrariumID" TEXT,
		"sensorID" TEXT,
		"value" TEXT,
		"timestamp" TEXT,
		"SessionKey" TEXT
	);`
	log.Println("Create measures table...")
	statement, err := db.Prepare(createMeasureTable) // Prepare SQL Statement
	if err != nil {
		log.Fatal(err.Error())
	}
	statement.Exec() // Execute SQL Statements
	log.Println("measures table created")
	/*Sensors*/
	createSensorTable := `CREATE TABLE sensors (
		"sensorID" TEXT NOT NULL PRIMARY KEY,
		"typeOfMeasure" TEXT,
		"extra_data" TEXT
	);`
	log.Println("Create Sensors table...")
	statement, err = db.Prepare(createSensorTable) // Prepare SQL Statement
	if err != nil {
		log.Fatal(err.Error())
	}
	statement.Exec() // Execute SQL Statements
	log.Println("Sensors table created")
	/*Terrariums*/
	createTerrariumsTable := `CREATE TABLE "terrariums" (
		"terrariumID"	TEXT NOT NULL,
		"typeOfTerrarium"	TEXT,
		"terrariumAlias"	TEXT,
		Status INTEGER NOT NULL DEFAULT '0',
		PRIMARY KEY("terrariumID")
	);`
	log.Println("Create Terrariums table...")
	statement, err = db.Prepare(createTerrariumsTable) // Prepare SQL Statement
	if err != nil {
		log.Fatal(err.Error())
	}
	statement.Exec() // Execute SQL Statements
	log.Println("Terrariums table created")

	createTerrariumsSensor := `
	CREATE TABLE "terrariumsSensors" (
		"terrariumID"	TEXT NOT NULL,
		"sensorID"	TEXT NOT NULL,
		PRIMARY KEY("sensorID","terrariumID")
	);`
	log.Println("Create Terrariums-Sensors table...")
	statement, err = db.Prepare(createTerrariumsSensor) // Prepare SQL Statement
	if err != nil {
		log.Fatal(err.Error())
	}
	statement.Exec() // Execute SQL Statements
	log.Println("Terrariums-Sensors table created")

	createTerrariumsLiveSession := `
	CREATE TABLE "terrariumsLiveSession" (
		"terrariumID"	TEXT NOT NULL,
		"SessionKey"	TEXT NOT NULL,
		"timestampStart" TEXT NOT NULL,
		"timestampEnd" TEXT NOT NULL,
		PRIMARY KEY("SessionKey","terrariumID")
	);`
	log.Println("Create Terrariums-Serrion table...")
	statement, err = db.Prepare(createTerrariumsLiveSession) // Prepare SQL Statement
	if err != nil {
		log.Fatal(err.Error())
	}
	statement.Exec() // Execute SQL Statements
	log.Println("Terrariums-Serrion table created")
}

func insertMeasureCheck(db *sql.DB, measure single_measure_data) bool {
	log.Println("Checking data consistency")
	sql_check_terrarium_existance := `SELECT terrariumID from terrariumsSensors
									  WHERE terrariumID = ?
									  AND sensorID = ?`
	statement, err := db.Prepare(sql_check_terrarium_existance) // Prepare statement.
	// This is good to avoid SQL injections
	if err != nil {
		log.Fatalln(err.Error())
	}
	var t_id string
	err = statement.QueryRow(measure.TerrariumID, measure.SensorID).Scan(&t_id)
	if err != nil {
		if err != sql.ErrNoRows {
			log.Fatalln(err.Error())
		} else {
			return false
		}
	}
	return true
}

func insertMeasure(db *sql.DB, measure single_measure_data) {
	log.Println("Inserting measure record")
	insertMeasureSQL := `INSERT INTO measures(terrariumID, sensorID, value, timestamp) VALUES (?,?, ?, ?)`
	statement, err := db.Prepare(insertMeasureSQL) // Prepare statement.
	// This is good to avoid SQL injections
	if err != nil {
		log.Fatalln(err.Error())
	}
	_, err = statement.Exec(measure.TerrariumID, measure.SensorID, measure.Value, measure.Timestamp)
	if err != nil {
		log.Fatalln(err.Error())
	}
}

/*Main dataDB init
 *Check presence or create DB
 *Check Table presence or create them
 */
func dataDBinit(dbPath string) *sql.DB {
	if !CheckDBFile(dbPath) {
		InitNewDBFile()
	}
	sqliteDatabase, _ := sql.Open("sqlite3", "./"+dbPath) //Open sqliteDB
	//Check db tables existence
	if !CheckDBTables(sqliteDatabase, _DB_TABLES) {
		CreateDBTables(sqliteDatabase)
	}
	return sqliteDatabase
}

/*Extract measures*/
func getMeasures(db *sql.DB, request measures_request_typ) []single_measure_data {
	var measures []single_measure_data
	/*Measures*/
	getMeasure := `SELECT terrariumID, sensorID, timestamp, value from measures where 
		terrariumID = ? AND 
		( timestamp BETWEEN ? AND ?) AND
		sensorID = ?
		ORDER BY timestamp
	;`

	statement, err := db.Prepare(getMeasure) // Prepare SQL Statement
	if err != nil {
		log.Fatal(err.Error())
	}
	row, err_r := statement.Query(request.TerrariumID, request.From, request.To, request.SensorID) // Execute SQL Statements
	if err_r != nil {
		log.Fatal((err_r.Error()))
	}
	for row.Next() {
		var measure single_measure_data
		row.Scan(&measure.TerrariumID, &measure.SensorID, &measure.Timestamp, &measure.Value)
		measures = append(measures, measure)
	}
	return measures
}

func createSessionRow(db *sql.DB, SessionKey string, terrariumID string, timestampStart string) string{
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
	return SessionKey;
}

func stopSession(db *sql.DB, SessionKey string, terrariumID string, timestampEnd string) bool{
	log.Println("Inserting session record")
	endSessionSQL := `UPDATE terrariumsLiveSession timestampEnd	= ? WHERE SessionKey = ? AND terrariumID = ? `
	statement, err := db.Prepare(endSessionSQL) // Prepare statement.
	// This is good to avoid SQL injections
	if err != nil {
		log.Fatalln(err.Error())
	}
	_, err = statement.Exec(timestampEnd, SessionKey, terrariumID)
	if err != nil {
		log.Fatalln(err.Error())
		return false;
	}
	return true;
}

func getTerrariums(db *sql.DB) []terrariumData {

	var t_terrariums []terrariumData

	t_query := `SELECT * from terrariums ;`

	statement, err := db.Prepare(t_query) // Prepare SQL Statement
	if err != nil {
		log.Fatal(err.Error())
	}

	row, err_r := statement.Query() // Execute SQL Statements
	if err_r != nil {
		log.Fatal((err_r.Error()))
	}

	for row.Next() {

		var t_terr terrariumData

		row.Scan(&t_terr.TerrariumID, &t_terr.TypeOfTerrarium, &t_terr.TerrariumAlias)

		t_sensors_query := `
		SELECT sensors.sensorID, sensors.typeOfMeasure, sensors.extra_data from sensors 
		INNER JOIN terrariumsSensors ON sensors.sensorID = terrariumsSensors.sensorID
		WHERE terrariumID = ?
		;`

		statement, err := db.Prepare(t_sensors_query) // Prepare SQL Statement
		if err != nil {
			log.Fatal(err.Error())
		}
		row_j, err_r := statement.Query(t_terr.TerrariumID) // Execute SQL Statements
		if err_r != nil {
			log.Fatal((err_r.Error()))
		}

		var t_sensors []sensorData

		for row_j.Next() {
			var t_sensor sensorData

			row_j.Scan(&t_sensor.SensorID, &t_sensor.TypeOfMeasure, &t_sensor.Extra_data)

			t_sensors = append(t_sensors, t_sensor)
		}

		t_terr.SensorsIds = t_sensors
		t_terrariums = append(t_terrariums, t_terr)

	}

	return t_terrariums
}
