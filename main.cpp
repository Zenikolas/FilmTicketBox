#include <iostream>

#include <Poco/Net/HTTPServer.h>

#include <Poco/Util/ServerApplication.h>

#include "handlers.h"

class CinemaServerApplication : public Poco::Util::ServerApplication {
public:
    void initialize(Application& self)
    {
        loadConfiguration(); // load default configuration files, if present
        ServerApplication::initialize(self);
    }

    void uninitialize()
    {
        ServerApplication::uninitialize();
    }

    int main(const std::vector<std::string> &args) override {
        // to change the port see filmTicketBox.properties config file
        unsigned int port = (unsigned int) config().getInt("filmTicketBox.port", 20323);
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
