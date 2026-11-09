#ifndef TRAINS_TRAINS_H
#define TRAINS_TRAINS_H

#include <array>
#include <compare>     // Used for std::strong_ordering.
#include <cstddef>     // Used for std::size_t type.
#include <ostream>
#include <string>
#include <string_view>
#include <utility>     // Used for std::move and std::exchange functions.

enum class WagonType { FIRST_CLASS, SECOND_CLASS, SLEEPER, DINING };

namespace trains_details {
    // Numbers of seats in the standard series.
    inline constexpr std::array<std::size_t, 4> STANDARD_SEATS{
        54, 80, 30, 32 };
    // Names of the wagon types used when printing. The order of the elements is
    // the same as the order of the values of WagonType.
    inline constexpr std::array<std::string_view, 4> TYPE_NAMES{
        "1 klasa", "2 klasa", "sypialny", "restauracyjny" };

    inline constexpr std::string_view UNKNOWN_DESIGNATION = "?";

    // Returns the position of the wagon type in the arrays above.
    constexpr std::size_t index_of(WagonType type) {
        return static_cast<std::size_t>(type);
    }

    constexpr std::size_t standard_seats(WagonType type) {
        return STANDARD_SEATS[index_of(type)];
    }

    constexpr std::string_view type_name(WagonType type) {
        return TYPE_NAMES[index_of(type)];
    }
} // namespace trains_details

// Series of wagons: the type and the number of seats.
class WagonSeries {
public:
    constexpr explicit WagonSeries(WagonType type, std::size_t seats)
        : type_{ type }, seats_{ seats } {
    }

    // Creates the standard series of the given type.
    constexpr WagonSeries(WagonType type)
        : WagonSeries{ type, trains_details::standard_seats(type) } {
    }

    WagonSeries() = delete;

    constexpr bool operator==(const WagonSeries&) const = default;

    constexpr WagonType type() const {
        return type_;
    }

    constexpr std::size_t seats() const {
        return seats_;
    }

private:
    WagonType type_;
    std::size_t seats_;
};

// Prints the series in the form [type, miejsca: number].
inline std::ostream& operator<<(std::ostream& os, const WagonSeries& series) {
    return os << '[' << trains_details::type_name(series.type())
              << ", miejsca: " << series.seats() << ']';
}

inline constexpr WagonSeries STANDARD_FIRST_CLASS{ WagonType::FIRST_CLASS };
inline constexpr WagonSeries STANDARD_SECOND_CLASS{ WagonType::SECOND_CLASS };
inline constexpr WagonSeries STANDARD_SLEEPER{ WagonType::SLEEPER };
inline constexpr WagonSeries STANDARD_DINING{ WagonType::DINING };

// Wagon with a unique number. The number 0 means that the object is empty,
// i.e. it does not represent any wagon.
class Wagon {
public:
    using unique_id_t = std::size_t;

    Wagon() = default;

    // Builds a new wagon of the given series with the next number.
    explicit Wagon(const WagonSeries& series)
        : id_{ ++ever_built_ }, series_{ series } {
        ++in_service_;
    }

    Wagon(const Wagon& other) {
        *this = other;
    }

    // Moving keeps the number and the series.
    Wagon(Wagon&& other) {
        *this = std::move(other);
    }

    // Rebuilds the wagon, so that it has the series of the other one.
    Wagon& operator=(const Wagon& other) {
        if (this == &other) {
            return *this;
        }
        if (!other) {
            release();
        }
        else {
            // A new wagon is built only when this object was empty.
            if (!*this) {
                id_ = ++ever_built_;
                ++in_service_;
            }
            series_ = other.series_;
        }
        return *this;
    }

    Wagon& operator=(Wagon&& other) {
        if (this != &other) {
            release();
            id_ = std::exchange(other.id_, 0);
            series_ = other.series_;
        }
        return *this;
    }

    ~Wagon() {
        release();
    }

    explicit operator bool() const {
        return id_ != 0;
    }

    operator WagonSeries() const {
        return series_;
    }

    unique_id_t number() const {
        return id_;
    }

    WagonSeries series() const {
        return series_;
    }

    bool operator==(const Wagon& other) const {
        return id_ == other.id_;
    }

    std::strong_ordering operator<=>(const Wagon& other) const {
        return id_ <=> other.id_;
    }

    static std::size_t in_service() {
        return in_service_;
    }

    static std::size_t ever_built() {
        return ever_built_;
    }

private:
    // Makes the object empty.
    void release() {
        if (id_ != 0) {
            --in_service_;
            id_ = 0;
        }
    }

    inline static constinit std::size_t ever_built_ = 0;
    inline static constinit std::size_t in_service_ = 0;

    unique_id_t id_ = 0;
    WagonSeries series_ = STANDARD_FIRST_CLASS;
};

inline std::ostream& operator<<(std::ostream& os, const Wagon& wagon) {
    if (!wagon) {
        return os << "Wagon pusty";
    }
    return os << "Wagon nr " << wagon.number() << ' ' << wagon.series();
}

// Locomotive with a designation and the maximal number of wagons it can pull.
class Locomotive {
public:
    Locomotive() = default;

    explicit Locomotive(std::string designation, std::size_t max_wagons)
        : designation_{ designation.empty()
                        ? std::string{ trains_details::UNKNOWN_DESIGNATION }
                        : std::move(designation) },
          max_wagons_{ max_wagons } {
    }

    // Locomotives cannot be copied.
    Locomotive(const Locomotive&) = delete;
    Locomotive& operator=(const Locomotive&) = delete;

    // Moving leaves the source empty.
    Locomotive(Locomotive&& other) {
        *this = std::move(other);
    }

    // The locomotive represented so far by this object stops existing.
    Locomotive& operator=(Locomotive&& other) {
        if (this != &other) {
            designation_ = std::move(other.designation_);
            max_wagons_ = std::exchange(other.max_wagons_, 0);
            other.designation_.clear();
        }
        return *this;
    }

    const std::string& designation() const {
        return designation_;
    }

    std::size_t max_wagons() const {
        return max_wagons_;
    }

    explicit operator bool() const {
        return !designation_.empty();
    }

private:
    std::string designation_;
    std::size_t max_wagons_ = 0;
};

inline std::ostream& operator<<(std::ostream& os, const Locomotive& loco) {
    if (!loco) {
        return os << "brak lokomotywy";
    }
    return os << loco.designation() << " (uciąg: " << loco.max_wagons() << ')';
}

#endif // TRAINS_TRAINS_H
