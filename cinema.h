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

class CinemaException : public std::exception {
    std::string m_reason;
public:
    explicit CinemaException(std::string_view reason) : m_reason(reason) {}

    const char *what() const throw() override {
        return m_reason.c_str();
    }
};

class CinemaSession {
    std::vector<std::vector<bool>> m_availableSeats;
    int m_avaliableSeatsCount;
    mutable std::shared_mutex m_mut;

    std::vector<std::string> getBusySeats(const std::vector<std::string> &bookingSeats);

public:
    CinemaSession(size_t width, size_t height);

    CinemaSession(CinemaSession &&rhs);

    CinemaSession &operator=(CinemaSession &&rhs);

    std::vector<std::string> availableSeats() const;

    bool bookSeat(size_t width, size_t height);

    std::vector<std::string> bookSeats(const std::vector<std::string> &bookingSeats);
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

    std::vector<std::string>
    checkAvailableSeats(const std::string &searchingFilm) const;

    bool bookSeat(const std::string &searchingFilm, size_t i, size_t j);

    std::vector<std::string> bookSeats(const std::string &searchingFilm, std::vector<std::string> bookingSeats);

    bool appendFilm(const std::string &filmName);
};

class Cinemas {
    std::unordered_map<std::string, Cinema> m_cinemas;
    mutable std::shared_mutex m_mut;
public:
    std::vector<std::string> listOfCinemas() const;

    std::vector<std::string> listOfFilms(const std::string &cinemaName) const;

    std::vector<std::string> listOfFilms() const;

    bool
    filmIsShowing(const std::string &cinemaName, const std::string &searchingFilm) const;

    std::vector<std::string>
    cinemasFilmIsShowing(const std::string &searchingFilm)
    const;

    std::vector<std::string> checkAvailableSeats(const std::string &cinemaName,
                                                 const std::string &searchingFilm) const;

    bool addCinema(std::string_view name, size_t width, size_t height);


    bool
    bookSeat(const std::string &cinemaName, const std::string &searchingFilm, size_t i,
             size_t j);

    std::vector<std::string>
    bookSeats(const std::string &cinemaName, const std::string &searchingFilm,
              const std::vector<std::string> &bookingSeats);

    bool appendFilm(const std::string &cinemaName, const std::string &filmName);
};

inline
CinemaSession::CinemaSession(size_t width, size_t height) : m_availableSeats(width),
                                                            m_avaliableSeatsCount(
                                                                    width * height) {
    for (auto &columnt : m_availableSeats) {
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
    m_availableSeats = std::move(rhs.m_availableSeats);
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

#endif //TESTPOCO_CINEMAS_H
