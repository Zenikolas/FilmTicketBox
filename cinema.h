#ifndef TESTPOCO_CINEMAS_H
#define TESTPOCO_CINEMAS_H

#include <assert.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <set>
#include <mutex>

class CinemaSession {
    std::vector<std::vector<bool>> m_seats;
    int m_avaliableSeatsCount;
    mutable std::shared_mutex m_mut;
public:
    CinemaSession(size_t width, size_t height);

    CinemaSession(CinemaSession &&rhs);

    CinemaSession &operator=(CinemaSession &&rhs);

    std::vector<std::string> availableSeats() const;

    bool bookSeat(size_t width, size_t height);
};

class Cinema {
public:
    std::unordered_map<std::string, CinemaSession> m_films;
    size_t m_width;
    size_t m_height;
    mutable std::shared_mutex m_mut;
public:
    Cinema(size_t width, size_t height);

    Cinema(Cinema &&rhs);

    Cinema &operator=(Cinema &&rhs);

    std::set<std::string> listOfFilms() const;

    bool filmIsShowing(const std::string &searchingFilm) const;

    std::vector<std::string> checkAvailableTickets(const std::string &searchingFilm) const;

    bool bookTicket(const std::string &searchingFilm, size_t i, size_t j);

    bool appendFilm(const std::string &filmName);
};

class Cinemas {
    std::unordered_map<std::string, Cinema> m_cinemas;
    mutable std::shared_mutex m_mut;
public:
    std::vector<std::string> listOfCinemas() const;

    std::set<std::string> listOfFilms(const std::string& cinemaName) const;
    std::set<std::string> listOfFilms() const;

    bool filmIsShowing(const std::string &cinemaName, const std::string &searchingFilm) const;

    std::vector<std::string> checkAvailableTickets(const std::string &cinemaName, const std::string &searchingFilm) const;

    bool addCinema(std::string_view name, size_t width, size_t height);


    bool bookTicket(const std::string &cinemaName, const std::string &searchingFilm, size_t i, size_t j);

    bool appendFilm(const std::string &cinemaName, const std::string &filmName);
};

inline
CinemaSession::CinemaSession(size_t width, size_t height) : m_seats(width),
                                                            m_avaliableSeatsCount(width * height) {
    for (auto &columnt : m_seats) {
        columnt.assign(height, true);
    }
}

inline
CinemaSession::CinemaSession(CinemaSession &&rhs) {
    this->operator=(std::move(rhs));
}

inline
CinemaSession &CinemaSession::operator=(CinemaSession &&rhs) {
    if (this == &rhs) {
        return *this;
    }

    std::scoped_lock lock(m_mut, rhs.m_mut);
    m_seats = std::move(rhs.m_seats);
    m_avaliableSeatsCount = rhs.m_avaliableSeatsCount;
}

inline
Cinema::Cinema(size_t width, size_t height) : m_width(width),
                                              m_height(height) {}

inline
Cinema::Cinema(Cinema &&rhs) {
    this->operator=(std::move(rhs));
}

inline
Cinema &Cinema::operator=(Cinema &&rhs) {
    if (this == &rhs) {
        return *this;
    }

    std::scoped_lock lock(m_mut, rhs.m_mut);
    m_films = std::move(rhs.m_films);
    m_width = rhs.m_width;
    m_height = rhs.m_height;
}

inline
std::set<std::string> Cinema::listOfFilms() const {
    std::set<std::string> films;
    {
        std::shared_lock lk(m_mut);
        for (auto &&film : m_films) {
            films.emplace(film.first);
        }
    }

    return films;
}

inline
bool Cinema::filmIsShowing(const std::string &searchingFilm) const {
    std::shared_lock lk(m_mut);
    return m_films.find(searchingFilm) != m_films.end();
}

inline
std::vector<std::string> Cinema::checkAvailableTickets(const std::string &searchingFilm) const {
    std::shared_lock lk(m_mut);
    auto it = m_films.find(searchingFilm);
    if (it == m_films.end()) {
        return {};
    }

    return it->second.availableSeats();
}

inline
bool Cinema::bookTicket(const std::string &searchingFilm, size_t i, size_t j) {
    std::shared_lock lk(m_mut);
    auto it = m_films.find(searchingFilm);
    if (it == m_films.end()) {
        return false;
    }

    return it->second.bookSeat(i, j);
}

inline
bool Cinema::appendFilm(const std::string &filmName) {
    std::lock_guard lk(m_mut);
    return m_films.emplace(filmName, CinemaSession(m_width, m_height)).second;
}

inline
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

inline
std::set<std::string> Cinemas::listOfFilms(const std::string& cinemaName) const {
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        return {};
    }

    return cinemaIt->second.listOfFilms();
}

std::set<std::string> Cinemas::listOfFilms() const {
    std::set<std::string> films;
    std::shared_lock lk(m_mut);
    for (auto& cinema : m_cinemas) {
        auto cinemaFilms = cinema.second.listOfFilms();
        films.insert(cinemaFilms.begin(), cinemaFilms.end());
    }

    return films;
}

inline
bool Cinemas::filmIsShowing(const std::string &cinemaName, const std::string &searchingFilm) const {
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        return false;
    }

    return cinemaIt->second.filmIsShowing(searchingFilm);
}

inline
std::vector<std::string> Cinemas::checkAvailableTickets(const std::string &cinemaName, const std::string &searchingFilm) const {
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        return {};
    }

    return cinemaIt->second.checkAvailableTickets(searchingFilm);
}

inline
bool Cinemas::addCinema(std::string_view name, size_t width, size_t height) {
    std::lock_guard lk(m_mut);
    return m_cinemas.emplace(name, Cinema(width, height)).second;
}

inline
bool Cinemas::bookTicket(const std::string &cinemaName, const std::string &searchingFilm, size_t i, size_t j)
{
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        return false;
    }

    return cinemaIt->second.bookTicket(searchingFilm, i, j);
}

inline
bool Cinemas::appendFilm(const std::string &cinemaName, const std::string &filmName) {
    std::shared_lock lk(m_mut);
    auto cinemaIt = m_cinemas.find(cinemaName);
    if (cinemaIt == m_cinemas.end()) {
        return false;
    }

    return cinemaIt->second.appendFilm(filmName);
}

#endif //TESTPOCO_CINEMAS_H
