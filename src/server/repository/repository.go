package repository

import (
	"context"

	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/mongo"
)

type Repository[T any] interface {
	FindOne(ctx context.Context, filter bson.M) (*T, error)
	Find(ctx context.Context, filter bson.M) ([]T, error)
	InsertOne(ctx context.Context, document T) (*mongo.InsertOneResult, error)
	UpdateOne(ctx context.Context, filter bson.M, update T) (*mongo.UpdateResult, error)
	DeleteOne(ctx context.Context, filter bson.M) (*mongo.DeleteResult, error)
}

type MongoDBRepository[T any] struct {
	collection *mongo.Collection
}

func NewMongoDBRepository[T any](collection *mongo.Collection) *MongoDBRepository[T] {
	return &MongoDBRepository[T]{collection: collection}
}

func (r *MongoDBRepository[T]) FindOne(ctx context.Context, filter bson.M) (*T, error) {
	var result T
	err := r.collection.FindOne(ctx, filter).Decode(&result)
	return &result, err
}

func (r *MongoDBRepository[T]) Find(ctx context.Context, filter bson.M) ([]T, error) {
	var results []T
	cur, err := r.collection.Find(ctx, filter)
	if err != nil {
		return nil, err
	}
	defer cur.Close(ctx)
	err = cur.All(ctx, &results)
	return results, err
}

func (r *MongoDBRepository[T]) InsertOne(ctx context.Context, document T) (*mongo.InsertOneResult, error) {
	return r.collection.InsertOne(ctx, document)
}

func (r *MongoDBRepository[T]) UpdateOne(ctx context.Context, filter bson.M, update T) (*mongo.UpdateResult, error) {
	return r.collection.UpdateOne(ctx, filter, bson.M{"$set": update})
}

func (r *MongoDBRepository[T]) DeleteOne(ctx context.Context, filter bson.M) (*mongo.DeleteResult, error) {
	return r.collection.DeleteOne(ctx, filter)
}
