#include "handlers.h"

#include <Poco/URI.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

namespace {
    std::ostream &sendReason(Poco::Net::HTTPServerResponse &response, const std::string &reason) {
        std::ostream &bodyStream = response.send();
        Poco::JSON::Object obj;
        obj.set("reason", reason);
        obj.stringify(bodyStream);
    }

    std::ostream &sendHTTPNotFound(Poco::Net::HTTPServerResponse &response, const std::string &reason = "") {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
        if (reason.empty()) {
            return response.send();
        } else {
            return sendReason(response, reason);
        }
    }

    std::ostream &sendHTTPMethodNotAllowed(Poco::Net::HTTPServerResponse &response, const std::string &reason = "") {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_METHOD_NOT_ALLOWED);
        if (reason.empty()) {
            return response.send();
        } else {
            return sendReason(response, reason);
        }
    }

    std::ostream &sendHTTPBadRequest(Poco::Net::HTTPServerResponse &response, const std::string &reason = "") {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
        if (reason.empty()) {
            return response.send();
        } else {
            return sendReason(response, reason);
        }
    }

    std::ostream &sendHTTPOKRequest(Poco::Net::HTTPServerResponse &response) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
        return response.send();
    }
}

void CinemasRequestHandler::handleCinemasRequest(Poco::Net::HTTPServerRequest &request,
                                                 Poco::Net::HTTPServerResponse &response) {
    std::istream &istream = request.stream();
    if (request.getMethod() == "POST") {
        if (request.getContentType() != "application/json") {
            sendHTTPBadRequest(response, "unsupported content-type");
            return;
        }
        if (addCinemas(istream)) {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_CREATED);
            response.send();
        } else {
            sendHTTPBadRequest(response, "Failed to add cinema");
            return;
        }
    } else if (request.getMethod() == "GET") {
        std::ostream &respBodyStream = sendHTTPOKRequest(response);
        std::vector<std::string> cinemas = m_cinemas.listOfCinemas();
        Poco::JSON::Object obj;
        obj.set("cinemas", cinemas);
        obj.stringify(respBodyStream);
    } else {
        sendHTTPMethodNotAllowed(response);
    }
}

void CinemasRequestHandler::handleCinemaRequest(Poco::Net::HTTPServerRequest &request,
                                                Poco::Net::HTTPServerResponse &response,
                                                std::vector<std::string> &pathSegments) {
    assert(pathSegments.size() == 2);

    if (request.getMethod() != "GET") {
        sendHTTPMethodNotAllowed(response);
        return;
    }

    const std::string &cinemaName = pathSegments[1];
    std::istream &istream = request.stream();
    if (cinemaName == "films") {
        auto films = m_cinemas.listOfFilms();
        std::ostream &respBodyStream = sendHTTPOKRequest(response);
        Poco::JSON::Object obj;
        obj.set("films", films);
        obj.stringify(respBodyStream);
        return;
    }

    auto cinemaFilms = m_cinemas.listOfFilms(cinemaName);
    std::ostream &respBodyStream = sendHTTPOKRequest(response);
    Poco::JSON::Object obj;
    obj.set("films", cinemaFilms);
    obj.stringify(respBodyStream);
}

void CinemasRequestHandler::handleFilmsRequest(Poco::Net::HTTPServerRequest &request,
                                               Poco::Net::HTTPServerResponse &response,
                                               std::vector<std::string> &pathSegments) {
    assert(pathSegments.size() == 3);
    const std::string &cinemaName = pathSegments[1];
    const std::string &film = pathSegments[2];

    if (request.getMethod() == "POST") {
        if (request.getContentType() != "application/json") {
            sendHTTPBadRequest(response, "Expected to POST application/json type");
            return;
        }

        if (!m_cinemas.filmIsShowing(cinemaName, film)) {
            sendHTTPBadRequest(response, "Film not found");
            return;
        }

        std::istream &istream = request.stream();
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(istream);
        if (result.isEmpty()) {
            sendHTTPBadRequest(response, "Invalid body format");
            return;
        }

        auto object = result.extract<Poco::JSON::Object::Ptr>();
        if (!object->has("seats")) {
            sendHTTPBadRequest(response, "Not found 'seats' key");
            return;
        }

        auto seats = object->getArray("seats");
        if (!seats) {
            sendHTTPBadRequest(response, "'seats' key value should be an array");
            return;
        }

        std::vector<std::string> seatsStr(seats->begin(), seats->end());
        std::vector<std::string> busySeats = m_cinemas.bookSeats(cinemaName, film, seatsStr);
        if (!busySeats.empty()) {
            std::ostream &respBodyStream = sendHTTPBadRequest(response);
            Poco::JSON::Object obj;
            obj.set("busy_seats", busySeats);
            obj.stringify(respBodyStream);
            return;
        }

        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_CREATED);
        response.send();
        return;
    }

    if (request.getMethod() != "GET") {
        sendHTTPMethodNotAllowed(response);
        return;
    }

    if (cinemaName == "films") {
        auto cinemas = m_cinemas.cinemasFilmIsShowing(film);
        std::ostream &respBodyStream = sendHTTPOKRequest(response);
        Poco::JSON::Object obj;
        obj.set("cinemas", cinemas);
        obj.stringify(respBodyStream);
        return;
    }

    if (!m_cinemas.filmIsShowing(cinemaName, film)) {
        sendHTTPBadRequest(response, "film not found");
        return;
    }

    auto availableSeats = m_cinemas.checkAvailableSeats(cinemaName, film);
    std::ostream &respBodyStream = sendHTTPOKRequest(response);
    Poco::JSON::Object obj;
    obj.set("seats", availableSeats);
    obj.stringify(respBodyStream);
}

bool CinemasRequestHandler::addCinemas(std::istream &content) {
    Poco::JSON::Parser parser; // static?
    Poco::Dynamic::Var result = parser.parse(content);
    if (result.isEmpty()) {
        std::cout << "No content!" << std::endl;
        return false;
    }
    auto object = result.extract<Poco::JSON::Object::Ptr>();
    if (!object->has("cinemas")) {
        return false;
    }

    auto cinemas = object->getArray("cinemas");
    if (!cinemas) {
        return false;
    }

    for (auto &cinema : *cinemas) {
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
        if (!m_cinemas.addCinema(cinemaName, width, height)) {
            return false;
        }

        auto films = cinemaObject->getArray("films");
        if (!films) {
            return false;
        }
        for (auto &filmVar: *films) {
            std::string film = filmVar;
            if (!m_cinemas.appendFilm(cinemaName, film)) {
                return false;
            }
        }
    }

    return true;
}

void
CinemasRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    Poco::URI uri(request.getURI());
    std::vector<std::string> pathSegments;
    uri.getPathSegments(pathSegments);
    response.setContentType("application/json");

    if (pathSegments.empty() || pathSegments[0] != "cinemas") {
        sendHTTPNotFound(response);
        return;
    }

    try {
        if (pathSegments.size() == 1) {
            handleCinemasRequest(request, response);
        } else if (pathSegments.size() == 2) {
            handleCinemaRequest(request, response, pathSegments);
        } else if (pathSegments.size() == 3) {
            handleFilmsRequest(request, response, pathSegments);
        } else {
            sendHTTPNotFound(response);
            return;
        }
    } catch (const std::runtime_error &exc) {
        std::ostream &bodyStream = sendHTTPBadRequest(response);
        Poco::JSON::Object obj;
        obj.set("reason", std::string(exc.what()));
        obj.stringify(bodyStream);
    }
}

Poco::Net::HTTPRequestHandler *CinemasHTTPRequestHandlerFactory::createRequestHandler(
        const Poco::Net::HTTPServerRequest &request) {
    return new CinemasRequestHandler(m_cinemas);
}
