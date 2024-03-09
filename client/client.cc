#include <string>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "HeartbeatMonitor.h"

ABSL_FLAG(std::string, host, "localhost:3000", "GRPC host address");
ABSL_FLAG(std::string, id, "unknown", "Heartbeat client ID");
ABSL_FLAG(int, sleep, 10000, "Sleep duration");

int main(int argc, char* argv[]) {
    absl::ParseCommandLine(argc, argv);
    std::string host = absl::GetFlag(FLAGS_host);
    std::string id = absl::GetFlag(FLAGS_id);
    int duration = absl::GetFlag(FLAGS_sleep);

    HeartbeatMonitor client(
        grpc::CreateChannel(host, grpc::InsecureChannelCredentials()),
        id
    );

    client.open();
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));

    return 0;
}