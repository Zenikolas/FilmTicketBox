#include <sstream>
#include "cinema.h"

namespace {
    std::string getPrintedSeat(size_t i, size_t j) {
        std::stringstream sstream;
        sstream << i << "row" << j << "seat";
        return sstream.str();
    }

    std::pair<int, int> getSeatFromPrinted(std::string_view seatStr) {
        int i = 0;
        int j = 0;
        if (sscanf(seatStr.data(), "%drow%dseat", &i, &j) == 2) {
            return {i, j};
        }

        if (sscanf(seatStr.data(), "%dx%d", &i, &j) == 2) {
            return {i, j};
        }

        throw CinemaException("bad format of requested seat");
    }
}

std::vector<std::string> CinemaSession::getBusySeats(const std::vector<std::string> &bookingSeats) {
    std::vector<std::string> busySeats;
    for (auto &seat : bookingSeats) {
        auto[i, j] = getSeatFromPrinted(seat);
        if (!m_availableSeats[i][j]) {
            busySeats.emplace_back(seat);
        }
    }

    return busySeats;
}

std::vector<std::string> CinemaSession::availableSeats() const { // todo: create cache
    std::vector<std::pair<int, int>> avaliableSeatsIdxs;
    avaliableSeatsIdxs.reserve(m_avaliableSeatsCount);
    {
        std::shared_lock lk(m_mut);
        for (size_t i = 0; i < m_availableSeats.size(); ++i) {
            for (size_t j = 0; j < m_availableSeats[i].size(); ++j) {
                if (m_availableSeats[i][j]) {
                    avaliableSeatsIdxs.emplace_back(i, j);
                }
            }
        }
    }

    assert(avaliableSeatsIdxs.size() == m_avaliableSeatsCount);
    std::vector<std::string> avaliableSeats;
    avaliableSeats.reserve(m_avaliableSeatsCount);
    for (auto &seat : avaliableSeatsIdxs) {
        avaliableSeats.emplace_back(getPrintedSeat(seat.first, seat.second));
    }

    return avaliableSeats;
}

bool CinemaSession::bookSeat(size_t width, size_t height) {
    bool booked = false;
    {
        std::lock_guard lk(m_mut);
        std::swap(m_availableSeats[width][height], booked);
    }


    if (booked) {
        --m_avaliableSeatsCount;
    }

    return booked;
}

std::vector<std::string> CinemaSession::bookSeats(const std::vector<std::string> &bookingSeats) {
    {
        std::shared_lock lk(m_mut);
        auto busySeats = getBusySeats(bookingSeats);
        if (!busySeats.empty()) {
            return busySeats;
        }
    }

    {
        std::lock_guard lk(m_mut);
        auto busySeats = getBusySeats(bookingSeats);
        if (!busySeats.empty()) {
            return busySeats;
        }

        for (auto &seat : bookingSeats) {
            auto[i, j] = getSeatFromPrinted(seat);
            m_availableSeats[i][j] = false;
        }

        return {};
    }
}

std::vector<std::string> Cinema::bookSeats(const std::string &searchingFilm, std::vector<std::string> bookingSeats) {
    std::shared_lock lk(m_mut);
    auto it = m_films.find(searchingFilm);
    if (it == m_films.end()) {
        throw CinemaException("film not found");
    }

    return it->second.bookSeats(bookingSeats);
}

std::set<std::string> Cinema::listOfFilms() const {
    std::set<std::string> films;
    {
        std::shared_lock lk(m_mut);
        for (auto &film : m_films) {
            films.emplace(film.first);
        }
    }

    return films;
}

bool Cinema::filmIsShowing(const std::string &searchingFilm) const {
    std::shared_lock lk(m_mut);
    return m_films.find(searchingFilm) != m_films.end();
}

std::vector<std::string>
Cinema::checkAvailableSeats(const std::string &searchingFilm) const {
    std::shared_lock lk(m_mut);
    auto it = m_films.find(searchingFilm);
    if (it == m_films.end()) {
        throw CinemaException("Film not found");
    }

    return it->second.availableSeats();
}

bool Cinema::bookSeat(const std::string &searchingFilm, size_t i, size_t j) {
    std::shared_lock lk(m_mut);
    auto it = m_films.find(searchingFilm);
    if (it == m_films.end()) {
        throw CinemaException("Film not found");
    }

    return it->second.bookSeat(i, j);
}

bool Cinema::appendFilm(const std::string &filmName) {
    std::lock_guard lk(m_mut);
    return m_films.emplace(filmName, CinemaSession(m_width, m_height)).second;
}

std::vector<std::string> Cinemas::listOfCinemas() const {
    std::vector<std::string> cinemas;
    cinemas.reserve(m_cinemas.size());
    {
        std::shared_lock lk(m_mut);
        for (auto &cinema : m_cinemas) {
            cinemas.emplace_back(cinema.first);
        }
    }

    return cinemas;
}

std::vector<std::string> Cinemas::listOfFilms(const std::string &cinemaName) const {
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        throw CinemaException("Cinema not found");
    }

    auto films = cinemaIt->second.listOfFilms();
    return std::vector<std::string>(films.begin(), films.end());
}

std::vector<std::string> Cinemas::listOfFilms() const {
    std::set<std::string> films;
    std::shared_lock lk(m_mut);
    for (auto &cinema : m_cinemas) {
        auto cinemaFilms = cinema.second.listOfFilms();
        films.insert(cinemaFilms.begin(), cinemaFilms.end());
    }

    return std::vector<std::string>(films.begin(), films.end());
}

bool Cinemas::filmIsShowing(const std::string &cinemaName,
                            const std::string &searchingFilm) const {
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        throw CinemaException("Cinema not found");
    }

    return cinemaIt->second.filmIsShowing(searchingFilm);
}

std::vector<std::string> Cinemas::cinemasFilmIsShowing(const std::string &
searchingFilm) const {
    std::vector<std::string> cinemas;
    std::shared_lock lk(m_mut);
    for (auto &cinema : m_cinemas) {
        if (cinema.second.filmIsShowing(searchingFilm)) {
            cinemas.emplace_back(cinema.first);
        }
    }

    return cinemas;
}

std::vector<std::string> Cinemas::checkAvailableSeats(const std::string &cinemaName,
                                                      const std::string &searchingFilm) const {
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        throw CinemaException("Cinema not found");
    }

    return cinemaIt->second.checkAvailableSeats(searchingFilm);
}

bool Cinemas::addCinema(std::string_view name, size_t width, size_t height) {
    std::lock_guard lk(m_mut);
    return m_cinemas.emplace(name, Cinema(width, height)).second;
}

bool Cinemas::bookSeat(const std::string &cinemaName, const std::string &searchingFilm,
                       size_t i, size_t j) {
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        throw CinemaException("Cinema not found");
    }

    return cinemaIt->second.bookSeat(searchingFilm, i, j);
}

bool Cinemas::appendFilm(const std::string &cinemaName, const std::string &filmName) {
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        throw CinemaException("Cinema not found");
    }

    return cinemaIt->second.appendFilm(filmName);
}

std::vector<std::string> Cinemas::bookSeats(const std::string &cinemaName, const std::string &searchingFilm,
                                            const std::vector<std::string> &bookingSeats) {
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        throw CinemaException("not found cinema");
    }

    return cinemaIt->second.bookSeats(searchingFilm, bookingSeats);
}