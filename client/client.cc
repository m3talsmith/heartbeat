#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "heartbeat.grpc.pb.h"

ABSL_FLAG(std::string, host, "localhost:3000", "GRPC host address");
ABSL_FLAG(std::string, id, "unknown", "Heartbeat client ID");

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientWriter;
using grpc::Status;
using heartbeat::Heartbeat;
using heartbeat::Beat;
using heartbeat::MonitorReply;

std::string get_timestamp() {
    std::chrono::time_point timestamp = std::chrono::system_clock::now();
    std::time_t tp = std::chrono::system_clock::to_time_t(timestamp);
    std::string ts = std::ctime(&tp);
    ts.resize(ts.size()-1);
    return ts;
}

void fatal(std::string msg) {
    std::string ts(get_timestamp());
    std::cerr << "[ERROR] " << ts << ": " << msg << std::endl;
    exit(1);
}

namespace heartbeat {
    namespace exception {
        class Connection : public std::exception {
            public:
                std::string what() {
                    return "bad connection";
                }
        };
        class Write : public std::exception {
            public:
                std::string what() {
                    return "bad write";
                }
        };
        class Response : public std::exception {
            public:
                std::string what() {
                    return "bad response";
                }
        };
    }
}

class HeartbeatClient {
    public:
        std::string id;
        bool closed;

        ClientContext context;
        MonitorReply reply;
        std::unique_ptr<ClientWriter<Beat>> writer;
        std::thread monitor_thread;

        HeartbeatClient(
            std::shared_ptr<Channel> channel,
            const std::string new_id
        ) : id(new_id), stub_(Heartbeat::NewStub(channel)) {}

        ~HeartbeatClient() {
            close();
        }

        void open() {
            std::cout << "opening monitor...";
            writer = stub_->Monitor(&context, &reply);
            monitor_thread = std::thread([this] {
                while (!closed) {
                    std::string ts(get_timestamp());

                    Beat beat;
                    beat.set_timestamp(ts);
                    beat.set_id(id);

                    if (!writer->Write(beat)) {
                        fatal(heartbeat::exception::Write().what());
                    }

                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(1000)
                    );
                }
                writer->WritesDone();
                Status status = writer->Finish();
                if (!status.ok()) {
                    fatal(heartbeat::exception::Response().what());
                }
            });
            std::cout << "done" << std::endl;
        }

        void close() {
            std::cout << "closing monitor...";
            closed = true;
            if (monitor_thread.joinable()) monitor_thread.join();
            std::cout << "done" << std::endl;
        }

    private:
        std::unique_ptr<Heartbeat::Stub> stub_;
};

int main(int argc, char* argv[]) {
    absl::ParseCommandLine(argc, argv);
    std::string host = absl::GetFlag(FLAGS_host);
    std::string id = absl::GetFlag(FLAGS_id);

    HeartbeatClient client(
        grpc::CreateChannel(host, grpc::InsecureChannelCredentials()),
        id
    );

    client.open();
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    return 0;
}