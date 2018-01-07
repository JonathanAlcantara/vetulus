#include <iostream>
#include <string>
#include <sstream>

#include <grpc++/grpc++.h>

#include "./service.grpc.pb.h"
#include "./spdlog/spdlog.h"

#include "./config.h"


using std::string;
using std::ostringstream;
using std::shared_ptr;

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;
using grpc::ServerBuilder;
using grpc::Server;
using grpc::ServerReader;

using VetulusService::ProtoFile;
using VetulusService::Ack;
using VetulusService::ProtoServer;


class ProtoServerImpl final : public ProtoServer::Service {
 private:
    shared_ptr<spdlog::logger> console;

 public:
    ProtoServerImpl() :ProtoServer::Service()
    {
        this->console = spdlog::get("Proto");
        if (!this->console) {
            this->console = spdlog::stdout_color_mt("Proto");
        }
        this->console->info("Proto Service");
    }

    Status Load(ServerContext* context, ServerReader<ProtoFile>* reader,
                Ack* ack) override
    {
        this->console->info("Load({0})", "test");
        return Status::OK;
    }
};


int
main(int argc, char* argv[])
{
    string config_file = "/etc/vetulus/services/proto_server.conf";

    if (argc > 1) {
        config_file = argv[1];
    }

    ProtoConfigLoader config;
    config.load(config_file);

    ostringstream ostr;
    ostr << config.addr << ":" << config.port;
    string serverAddress(ostr.str());

    ProtoServerImpl service;
    ServerBuilder builder;

    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    spdlog::get("Proto")->info("Listening on port {0}", serverAddress);

    server->Wait();

    return EXIT_SUCCESS;
}
