#ifndef FILMTICKETBOX_HANDLERS_H
#define FILMTICKETBOX_HANDLERS_H

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPRequestHandler.h>

#include "cinema.h"

class CinemasRequestHandler : public Poco::Net::HTTPRequestHandler {
    Cinemas &m_cinemas;

    bool addCinemas(std::istream &content);

    void handleCinemasRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

    void handleCinemaRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response,
                             std::vector<std::string> &pathSegments);

    void handleFilmsRequest(Poco::Net::HTTPServerRequest &request,
                            Poco::Net::HTTPServerResponse &response,
                            std::vector<std::string> &pathSegments);

public:
    explicit CinemasRequestHandler(Cinemas &cinemas) : m_cinemas(cinemas) {}

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
};

class CinemasHTTPRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    Cinemas m_cinemas;
public:
    Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
};


#endif //FILMTICKETBOX_HANDLERS_H
