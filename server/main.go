package main

import (
	"flag"
	"fmt"
	"io"
	"log"
	"net"
	"time"

	pb "heartbeat/heartbeat"

	"google.golang.org/grpc"
)

var (
	port = flag.Int("port", 3000, "The server port")
)

type service struct {
	pb.UnimplementedHeartbeatServer
}

func (s *service) Monitor(stream pb.Heartbeat_MonitorServer) error {
	id := "unknown"
	lastseen := time.Now().GoString()
	for {
		beat, err := stream.Recv()
		if err != nil {
			if err == io.EOF {
				log.Printf("[HEARTBEAT:OFFLINE] %s: %s", id, lastseen)
				return nil
			}
			log.Printf("[HEARTBEAT:ERROR] %s: %s: %v", id, lastseen, err)
			return err
		}
		id = beat.Id
		lastseen = beat.Timestamp
		log.Printf("[HEARTBEAT:ONLINE] %s: %s", id, lastseen)
	}
}

func main() {
	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf(":%d", *port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	s := grpc.NewServer()
	pb.RegisterHeartbeatServer(s, &service{})
	log.Printf("server listening at %v", lis.Addr())
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
