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

type MeasureData struct {
	terrariumID string
	SensorID    string
	Value       string
	Timestamp   string
}

type terrariumData struct {
	terrariumID     string
	typeOfTerrarium string
}

type sensorData struct {
	sensorID      string
	typeOfMeasure string
	extra_data    string
}

type request_data struct {
	Data []MeasureData
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

func insertMeasure(db *sql.DB, sensorID string, value string, timestamp string) {
	log.Println("Inserting measure record")
	insertMeasureSQL := `INSERT INTO measures(sensorID, value, timestamp) VALUES (?, ?, ?)`
	statement, err := db.Prepare(insertMeasureSQL) // Prepare statement.
	// This is good to avoid SQL injections
	if err != nil {
		log.Fatalln(err.Error())
	}
	_, err = statement.Exec(sensorID, value, timestamp)
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
