#ifndef HEARTBEATMONITOR_H
#define HEARTBEATMONITOR_H

#include <grpc/grpc.h>
#include <grpcpp/channel.h>

#include "heartbeat.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientWriter;

using heartbeat::Heartbeat;
using heartbeat::Beat;
using heartbeat::MonitorReply;

class HeartbeatMonitor {
    public:
        std::string id;
        bool closed;

        ClientContext context;
        MonitorReply reply;
        std::unique_ptr<ClientWriter<Beat>> writer;
        std::thread monitor_thread;

        HeartbeatMonitor(
            std::shared_ptr<Channel> channel,
            const std::string new_id
        );
        ~HeartbeatMonitor();

        void open();
        void close();
    private:
        std::unique_ptr<Heartbeat::Stub> stub_;
};

#endif //HEARTBEATMONITOR_H
