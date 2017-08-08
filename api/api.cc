/*
 * ============================================================================
 *
 *       Filename:  main_api.cpp
 *
 *    Description:  Main execution of the API server
 *
 *        Version:  1.0
 *        Created:  07/26/2017 12:47:30 AM
 *       Compiler:  g++
 *
 *         Author:  Gustavo Pantuza (gustavopantuza@gmail.com),
 *   Organization:  Computer science community
 *
 * ============================================================================
 */


#include <pistache/endpoint.h>

#include "api.h"


using namespace Pistache;


VetulusAPI::VetulusAPI (APIConfigLoader config)
    : port(Port(stoi(config.port)))
    , addr(config.addr, port)
    , threads(stoi(config.threads))
    , endpoint(make_shared<Http::Endpoint>(addr))
{
    this->configure();
    this->setRoutes();
    this->setListeners();
}


void VetulusAPI::configure ()
{
    auto opts = Http::Endpoint::options().threads(this->threads).flags(
            Tcp::Options::InstallSignalHandler);

	this->endpoint->init(opts);
}


void VetulusAPI::setListeners ()
{
    signal(SIGINT, VetulusAPI::sigintCallback);
}


static void VetulusAPI::sigintCallback (int signal)
{
    cout << "Caught keyboard interruption. Shuting down Vetulus API" << endl;
    exit(1);
}


void VetulusAPI::simpleResponse (const Rest::Request& request, Http::ResponseWriter response)
{
     response.send(Http::Code::Ok, "{\"status\": 200, \"body\": \"Hello, World\"}");
}


void VetulusAPI::setRoutes ()
{
    using namespace Rest;

    Routes::Get(
        this->router,
        "/",
        Routes::bind(&VetulusAPI::simpleResponse, this)
    );

    /*
     * Every new API goes here. A new class should implement the setRoutes
     * method to assign its own routes and handlers callbacks
     */
    this->info_handler.setRoutes(this->router);
}


void VetulusAPI::listen ()
{
	this->endpoint->setHandler(this->router.handler());

    cout << "Vetulus API listening at " << this->addr.host()
         << ":" << this->addr.port() << " with "
         << this->threads << " threads" << endl;

    this->endpoint->serve();
    this->endpoint->shutdown();
}


int main(int argc, char* argv[]) {

    string config_file = "/etc/vetulus/api.conf";

    if(argc > 1) {
        config_file = argv[1];
    }

    APIConfigLoader config;
    config.load(config_file);

    VetulusAPI server(config);

    server.listen();
}
