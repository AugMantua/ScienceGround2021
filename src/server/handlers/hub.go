package handlers

import (
	"log"
)

// Hub maintains the set of active clients and broadcasts messages to the
// clients.
type Hub struct {
	// Registered Clients.
	Clients map[*Client]bool

	// Inbound messages from the clients.
	Broadcast chan []Single_measure_data

	// Register requests from the clients.
	Register chan *Client

	// Unregister requests from clients.
	Unregister chan *Client
}

func NewHub() *Hub {
	return &Hub{
		Broadcast:  make(chan []Single_measure_data),
		Register:   make(chan *Client),
		Unregister: make(chan *Client),
		Clients:    make(map[*Client]bool),
	}
}

func (h *Hub) Run() {
	for {
		select {
		case client := <-h.Register:
			log.Println("Client registered for terrarium ID :", client.terrariumId, "data stream")
			h.Clients[client] = true
		case client := <-h.Unregister:
			if _, ok := h.Clients[client]; ok {
				delete(h.Clients, client)
				close(client.send)
				log.Println("Client stream with terrarium ID :", client.terrariumId, "disconnected")
			}
		case measures := <-h.Broadcast:
			for client := range h.Clients {
				if measures[0].TerrariumID.Hex() == client.terrariumId {
					select {
					case client.send <- measures:
					default:
						close(client.send)
						delete(h.Clients, client)
					}
				}
			}
		}
	}
}
