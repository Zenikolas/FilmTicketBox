#include <iostream>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include <Poco/Util/ServerApplication.h>

#include <Poco/StreamCopier.h>

#include <chrono>

#include "cinema.h"

Poco::DateTime getCurrentTime() {
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    return Poco::DateTime(*timeinfo);
}

class MyHTTPRequestHandler : public Poco::Net::HTTPRequestHandler {
    Cinemas& m_cinemas;

public:
    MyHTTPRequestHandler(Cinemas& cinemas) : m_cinemas(cinemas) {}

    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override {
        auto uri = request.getURI();
        std::istream& istream = request.stream();
        std::cout << "URI: " << uri << std::endl;

        if (request.getMethod() == "POST") {
            // todo
        } else if (request.getMethod() == "GET"){
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
            std::ostream& respBodyStream = response.send();
            Poco::JSON::Object obj;

            Poco::DateTime now = getCurrentTime();
            obj.set("requested_time", now);
            obj.stringify(respBodyStream);
        } else {
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
            // todo add error description
            response.send();
        }







    }
};

class CinemasRequestHandler : public Poco::Net::HTTPRequestHandler {
    Cinemas& m_cinemas;

    bool addCinemas(std::istream& content);
public:
    CinemasRequestHandler(Cinemas& cinemas) : m_cinemas(cinemas) {}

    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override {
        std::istream& istream = request.stream();
        if (request.getMethod() == "POST") {
            if (addCinemas(istream)) {
                response.setStatus(Poco::Net::HTTPServerResponse::HTTP_CREATED);
                response.send();
            } else {
                response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
                response.send();
            }
        } else if (request.getMethod() == "GET"){
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
            std::ostream& respBodyStream = response.send();
            std::vector<std::string> cinemas = m_cinemas.listOfCinemas();
            Poco::JSON::Object obj;
            obj.set("cinemas", cinemas);
            obj.stringify(respBodyStream);
        } else {
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
            response.send();
            // todo add error description
        }







    }
};

bool CinemasRequestHandler::addCinemas(std::istream& content)
{
    Poco::JSON::Parser parser; // static?
    Poco::Dynamic::Var result = parser.parse(content);
    if (result.isEmpty()) {
        std::cout << "No content!" << std::endl;
        return false;
    }
    auto object = result.extract<Poco::JSON::Object::Ptr>();
    std::cout << "Content:\n";
    object->stringify(std::cout);
    std::cout << std::endl;

    if (!object->has("cinemas")) {
        return false; //todo add error message
    }

    auto cinemas = object->getArray("cinemas");
    if (!cinemas) {
        return false;
    }

    for (auto& cinema : *cinemas) {

        std::cout << "Cinema:" << std::endl;
        auto cinemaObject = cinema.extract<Poco::JSON::Object::Ptr>();
        if (cinemaObject.isNull()) {
            return false;
        }

        if (!cinemaObject->has("name")) {
            return false;
        }
        const std::string cinemaName = cinemaObject->get("name");

        if (!cinemaObject->has("width")) {
            return false;
        }
        int width = cinemaObject->get("width");

        if (!cinemaObject->has("height")) {
            return false;
        }
        int height = cinemaObject->get("height");
        m_cinemas.addCinema(cinemaName, width, height);

        auto films = cinemaObject->getArray("films");
        if (!films) {
            return false;
        }
        for (auto& filmVar: *films) {
            std::string film = filmVar;
            m_cinemas.appendFilm(cinemaName, film); //todo check return value
        }
    }


    return true;
}

class MyHTTPRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    Cinemas m_cinemas;
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override {
        auto& uri = request.getURI();
        std::cout << "Requested URI: " << uri << std::endl;
        if (uri == "/cinemas" || uri == "/cinemas/") {
            new CinemasRequestHandler(m_cinemas);
        } else {
            return new MyHTTPRequestHandler(m_cinemas);
        }

    }


};

class MyServerApplication: public Poco::Util::ServerApplication {
    int main(const std::vector<std::string>& args) override {
        size_t port = 20322;
        Poco::Net::HTTPServer httpServer(new MyHTTPRequestHandlerFactory, port);

        httpServer.start();

        std::cout << "Listening on 127.0.0.1:" << httpServer.port() << std::endl;

        Poco::Util::ServerApplication::waitForTerminationRequest();

        httpServer.stop();
    }
};


int main(int argc, char* argv[]) {

    MyServerApplication app;
    return app.run(argc, argv);
}
