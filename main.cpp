#include <iostream>

#include <Poco/Net/HTTPServer.h>

#include <Poco/Util/ServerApplication.h>

#include "handlers.h"

class CinemaServerApplication : public Poco::Util::ServerApplication {
    int main(const std::vector<std::string> &args) override {
        size_t port = 20322; //add port ot args and  check for hostname
        Poco::Net::HTTPServer httpServer(new CinemasHTTPRequestHandlerFactory, port);

        httpServer.start();

        std::cout << "Listening on 127.0.0.1:" << httpServer.port() << std::endl;

        Poco::Util::ServerApplication::waitForTerminationRequest();

        httpServer.stop();
    }
};


int main(int argc, char *argv[]) {
    CinemaServerApplication app;
    return app.run(argc, argv);
}
