package utils

import (
	"context"
	"fmt"
	"testing"

	"github.com/testcontainers/testcontainers-go"
	"github.com/testcontainers/testcontainers-go/wait"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
)

func CreateTestMongoDBClient(ctx context.Context, t *testing.T) (*mongo.Client, func()) {
	req := testcontainers.ContainerRequest{
		Image:        "mongo:4.2",
		ExposedPorts: []string{"27017/tcp"},
		WaitingFor:   wait.ForListeningPort("27017/tcp"),
	}
	mongoC, err := testcontainers.GenericContainer(ctx, testcontainers.GenericContainerRequest{
		ContainerRequest: req,
		Started:          true,
	})
	if err != nil {
		t.Fatalf("Failed to create container: %v", err)
	}

	mongoHost, err := mongoC.Host(ctx)
	if err != nil {
		t.Fatalf("Failed to get container host: %v", err)
	}

	mongoPort, err := mongoC.MappedPort(ctx, "27017")
	if err != nil {
		t.Fatalf("Failed to get container port: %v", err)
	}

	mongoURL := fmt.Sprintf("mongodb://%s:%s", mongoHost, mongoPort.Port())

	client, err := mongo.Connect(ctx, options.Client().ApplyURI(mongoURL))
	if err != nil {
		t.Fatalf("Failed to connect to MongoDB: %v", err)
	}

	return client, func() {
		client.Disconnect(ctx)
		mongoC.Terminate(ctx)
	}
}
