#include "handlers.h"

#include <Poco/URI.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

namespace {
void sendHTTPBadRequest(Poco::Net::HTTPServerResponse &response)
{
    response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
    response.send();
}
}

void CinemasRequestHandler::handleCinemasRequest(Poco::Net::HTTPServerRequest &request,
                                                 Poco::Net::HTTPServerResponse &response) {
    std::istream &istream = request.stream();
    if (request.getMethod() == "POST") {
        if (request.getContentType() != "application/json") {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
            response.send();
            return;
        }
        if (addCinemas(istream)) {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_CREATED);
            response.send();
        } else {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
            response.send();
        }
    } else if (request.getMethod() == "GET") {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
        response.setContentType("application/json");
        std::ostream &respBodyStream = response.send();
        std::vector<std::string> cinemas = m_cinemas.listOfCinemas();
        Poco::JSON::Object obj;
        obj.set("cinemas", cinemas);
        obj.stringify(respBodyStream);
    } else {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_METHOD_NOT_ALLOWED);
        response.send();
    }
}

void CinemasRequestHandler::handleCinemaRequest(Poco::Net::HTTPServerRequest &request,
                                                Poco::Net::HTTPServerResponse &response, std::vector<std::string>& pathSegments) {
    assert(pathSegments.size() == 2);

    if (request.getMethod() != "GET") {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_METHOD_NOT_ALLOWED);
        response.send();
        return;
    }

    const std::string& cinemaName = pathSegments[1];
    std::istream &istream = request.stream();
    if (cinemaName == "films") {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
        response.setContentType("application/json");
        std::ostream &respBodyStream = response.send();
        std::set<std::string> films = m_cinemas.listOfFilms();
        Poco::JSON::Object obj;
        obj.set("films", films);
        obj.stringify(respBodyStream);
        return;
    }

    std::set<std::string> films = m_cinemas.listOfFilms(cinemaName);
    if (films.empty()) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
        response.send();
        return;
    }

    response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
    response.setContentType("application/json");
    std::ostream &respBodyStream = response.send();
    Poco::JSON::Object obj;
    obj.set("films", films);
    obj.stringify(respBodyStream);
}

void CinemasRequestHandler::handleFilmsRequest(Poco::Net::HTTPServerRequest &request,
                                           Poco::Net::HTTPServerResponse &response,
                                                                      std::vector<std::string> &pathSegments) {
    assert(pathSegments.size() == 3);
    const std::string& cinemaName = pathSegments[1];
    const std::string& film = pathSegments[2];

    if (request.getMethod() == "POST") {
        if (request.getContentType() != "application/json") {
            return sendHTTPBadRequest(response);
        }

        if (!m_cinemas.filmIsShowing(cinemaName, film)) {
            return sendHTTPBadRequest(response);
        }

        std::istream &istream = request.stream();
        Poco::JSON::Parser parser; // combine this to one method
        Poco::Dynamic::Var result = parser.parse(istream);
        if (result.isEmpty()) {
            std::cout << "No content!" << std::endl;
            return sendHTTPBadRequest(response);
        }

        auto object = result.extract<Poco::JSON::Object::Ptr>();
        if (!object->has("seats")) {
            return sendHTTPBadRequest(response);
        }

        auto seats = object->getArray("seats");
        if (!seats) {
            return sendHTTPBadRequest(response);
        }

        for (auto &seat : *seats) {
            const std::string seatStr = seat;

        }




        return;
    }

    if (request.getMethod() != "GET") {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_METHOD_NOT_ALLOWED);
        response.send();
        return;
    }

    if (cinemaName == "films") {
        auto cinemas = m_cinemas.cinemasFilmIsShowing(film);
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
        response.setContentType("application/json");
        std::ostream &respBodyStream = response.send();
        Poco::JSON::Object obj;
        obj.set("cinemas", cinemas);
        obj.stringify(respBodyStream);
        return;
    }

    if (!m_cinemas.filmIsShowing(cinemaName, film)) {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
        response.send();
        return;
    }

    auto seats = m_cinemas.checkAvailableSeats(cinemaName, film);
    response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_OK);
    response.setContentType("application/json");
    std::ostream &respBodyStream = response.send();
    Poco::JSON::Object obj;
    obj.set("seats", seats);
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
//    std::cout << "Content:\n";
//    object->stringify(std::cout); //todo move it to log level debug
//    std::cout << std::endl;

    if (!object->has("cinemas")) {
        return false;
    }

    auto cinemas = object->getArray("cinemas");
    if (!cinemas) {
        return false;
    }

    for (auto &cinema : *cinemas) {

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
        for (auto &filmVar: *films) {
            std::string film = filmVar;
            m_cinemas.appendFilm(cinemaName, film); //todo check return value
        }
    }

    return true;
}

void CinemasRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    Poco::URI uri(request.getURI());
    std::vector<std::string> pathSegments;
    uri.getPathSegments(pathSegments);

    if (pathSegments.empty() || pathSegments[0] != "cinemas") {
        response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
        response.send();
        return;
    }

    if (pathSegments.size() == 1) {
        handleCinemasRequest(request, response);
    } else if (pathSegments.size() == 2) {
        handleCinemaRequest(request, response, pathSegments);
    } else if (pathSegments.size() == 3) {
        handleFilmsRequest(request, response, pathSegments);
    } else {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
        response.send();
    }
}

Poco::Net::HTTPRequestHandler * CinemasHTTPRequestHandlerFactory::createRequestHandler(
        const Poco::Net::HTTPServerRequest &request) {
    return new CinemasRequestHandler(m_cinemas);
}
