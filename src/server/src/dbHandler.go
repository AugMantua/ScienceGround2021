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
}

type terrariumData struct {
	TerrariumID     string
	TypeOfTerrarium string
}

type sensorData struct {
	SensorID      string
	TypeOfMeasure string
	Extra_data    string
}

type single_measure_data struct {
	TerrariumID string
	SensorID    string
	Value       string
	Timestamp   string
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
		"timestamp" TEXT
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
		"sensorID" integer NOT NULL PRIMARY KEY,
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
	createTerrariumsTable := `CREATE TABLE terrariums (
		"terrariumID" TEXT NOT NULL PRIMARY KEY,
		"typeOfTerrarium" TEXT
	);`
	log.Println("Create Terrariums table...")
	statement, err = db.Prepare(createTerrariumsTable) // Prepare SQL Statement
	if err != nil {
		log.Fatal(err.Error())
	}
	statement.Exec() // Execute SQL Statements
	log.Println("Terrariums table created")
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
		( timestamp BETWEEN ? AND ?)
		ORDER BY timestamp
	;`
	statement, err := db.Prepare(getMeasure) // Prepare SQL Statement
	if err != nil {
		log.Fatal(err.Error())
	}
	row, err_r := statement.Query(request.TerrariumID, request.From, request.To) // Execute SQL Statements
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

		row.Scan(&t_terr.TerrariumID, &t_terr.TypeOfTerrarium)
		t_terrariums = append(t_terrariums, t_terr)

	}

	return t_terrariums
}