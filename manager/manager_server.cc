#include <iostream>

#include <string>
#include <sstream>
#include <fstream>

#include <grpc++/grpc++.h>

#include "./service.grpc.pb.h"
#include "./spdlog/spdlog.h"

#include "./config.h"
#include "./proto_loader.h"


using std::string;
using std::ifstream;
using std::stringstream;
using std::ostringstream;
using std::shared_ptr;

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;
using grpc::ServerBuilder;
using grpc::Server;

using VetulusService::ProtoFile;
using VetulusService::Ack;
using VetulusService::Manager;
using VetulusService::MetaData;

using manager::VetulusProtoBuilder;


class ManagerServer final : public Manager::Service {
 private:
    shared_ptr<spdlog::logger> console;

 public:
    ManagerServer() :Manager::Service()
    {
        this->console = spdlog::get("Proto");
        if (!this->console) {
            this->console = spdlog::stdout_color_mt("Proto");
        }
        this->console->info("Proto Service");
    }

    Status Load(ServerContext* context, const ProtoFile* proto,
                Ack* ack) override
    {
        string file_name = proto->meta().name() + ".proto";
        this->console->info("Load({0})", file_name);

        /* If file already exists we do not upload it */
        ifstream infile(file_name.c_str());
        if (infile.good()) {
            ack->set_done(false);
            this->console->error("Could not load file {0}. "
                                 "File already exists", file_name);
            return Status::OK;
        } else {
            ofstream proto_file(file_name.c_str());
            proto_file << proto->data();
            proto_file.close();

            VetulusProtoBuilder builder;
            if (builder.Import(file_name)) {
                if (builder.CppGenerate()) {
                    ack->set_done(true);
                    return Status::OK;
                }
            }
            ack->set_done(false);
            return Status::OK;
        }
    }

    Status Unload(ServerContext* context, const MetaData* meta,
                  Ack* ack) override
    {
        string proto_name = meta->name();
        string proto = proto_name + ".proto";
        string header = proto_name + ".pb.h";
        string source = proto_name + ".pb.cc";

        bool done = true;
        if (remove(proto.c_str()) != 0) {
            done = false;
            this->console->error("Remove({0}): fail", proto);
        }
        if (remove(header.c_str()) != 0) {
            done = false;
            this->console->error("Remove({0}): fail", header);
        }
        if (remove(source.c_str()) != 0) {
            done = false;
            this->console->error("Remove({0}): fail", source);
        }
        ack->set_done(done);
        return Status::OK;
    }
};


int
main(int argc, char* argv[])
{
    string config_file = "/etc/vetulus/services/manager_server.conf";

    if (argc > 1) {
        config_file = argv[1];
    }

    ProtoConfigLoader config;
    config.load(config_file);

    ostringstream ostr;
    ostr << config.addr << ":" << config.port;
    string serverAddress(ostr.str());

    ManagerServer service;
    ServerBuilder builder;

    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    spdlog::get("Proto")->info("Listening on port {0}", serverAddress);

    server->Wait();

    return EXIT_SUCCESS;
}
