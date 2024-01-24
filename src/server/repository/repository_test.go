package repository

import (
	"context"
	"testing"

	"server/utils"

	"go.mongodb.org/mongo-driver/bson"
)

type TestDocument struct {
	ID   string `bson:"_id,omitempty"`
	Name string `bson:"name,omitempty"`
}

func TestMongoDBRepository(t *testing.T) {
	ctx := context.Background()
	client, cleanup := utils.CreateTestMongoDBClient(ctx, t)
	defer cleanup()

	repo := NewMongoDBRepository[TestDocument](client.Database("test").Collection("test"))

	t.Run("Test InsertOne", func(t *testing.T) {
		doc := TestDocument{Name: "Test"}
		_, err := repo.InsertOne(ctx, doc)
		if err != nil {
			t.Errorf("Failed to insert document: %v", err)
		}
	})

	t.Run("Test FindOne", func(t *testing.T) {
		result, err := repo.FindOne(ctx, bson.M{"name": "Test"})
		if err != nil {
			t.Errorf("Failed to find document: %v", err)
		}
		if result.Name != "Test" {
			t.Errorf("Expected name 'Test', got '%s'", result.Name)
		}
	})

	t.Run("Test UpdateOne", func(t *testing.T) {
		_, err := repo.UpdateOne(ctx, bson.M{"name": "Test"}, TestDocument{Name: "Updated"})
		if err != nil {
			t.Errorf("Failed to update document: %v", err)
		}
	})

	t.Run("Test Find after update", func(t *testing.T) {
		result, err := repo.FindOne(ctx, bson.M{"name": "Updated"})
		if err != nil {
			t.Errorf("Failed to find document: %v", err)
		}
		if result.Name != "Updated" {
			t.Errorf("Expected name 'Updated', got '%s'", result.Name)
		}
	})
}
