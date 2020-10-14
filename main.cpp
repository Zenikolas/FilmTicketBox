#include <iostream>

#include <Poco/Net/HTTPServer.h>

#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/IntValidator.h>

#include "handlers.h"

class CinemaServerApplication : public Poco::Util::ServerApplication {
    bool m_helpRequested = false;
    std::optional<unsigned int> m_port;
public:
    void initialize(Application &self) {
        loadConfiguration(); // load default configuration files, if present
        ServerApplication::initialize(self);
    }

    void uninitialize() {
        ServerApplication::uninitialize();
    }

    void defineOptions(Poco::Util::OptionSet &options) override {
        ServerApplication::defineOptions(options);

        options.addOption(
                Poco::Util::Option("help", "h", "display help information on command line arguments")
                        .required(false)
                        .repeatable(false));

        options.addOption(
                Poco::Util::Option("port", "p",
                                   "specify port to run on, have higher priority than port from config file", false,
                                   "port-value", true)
                        .repeatable(false)
                        .validator(new Poco::Util::IntValidator(1024, 65535)));
    }

    void handleOption(const std::string &name, const std::string &value) override {
        ServerApplication::handleOption(name, value);

        if (name == "help") {
            m_helpRequested = true;
        } else if (name == "port") {
            m_port = std::stoi(value);
        }
    }

    void displayHelp() {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("A cinema booking tickets server implemented using REST API style.");
        helpFormatter.format(std::cout);
    }

    int main(const std::vector<std::string> &args) override {
        if (m_helpRequested) {
            displayHelp();
            return Poco::Util::Application::EXIT_USAGE;
        }

        // to change the port see filmTicketBox.properties config file
        const unsigned int DEFAULT_PORT = 20322;
        unsigned int port = m_port ? m_port.value() : static_cast<unsigned int>(config().getInt("filmTicketBox.port",
                                                                                                DEFAULT_PORT));
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
