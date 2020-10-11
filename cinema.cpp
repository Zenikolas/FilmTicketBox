#include <sstream>
#include "cinema.h"

namespace {
    std::string getPrintedSeat(size_t i, size_t j) {
        std::stringstream sstream;
        sstream << i << "row" << j << "place";
    }
}

std::vector<std::string> CinemaSession::availableSeats() const { // todo: create cache
    std::vector<std::pair<int, int>> avaliableSeatsIdxs;
    avaliableSeatsIdxs.reserve(m_avaliableSeatsCount);
    {
        std::shared_lock lk(m_mut);
        for (size_t i = 0; i < m_seats.size(); ++i) {
            for (size_t j = 0; j < m_seats[i].size(); ++j) {
                if (m_seats[i][j]) {
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
        std::swap(m_seats[width][height], booked);
    }


    if (booked) {
        --m_avaliableSeatsCount;
    }

    return booked;
}
