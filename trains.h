#ifndef TRAINS_TRAINS_H
#define TRAINS_TRAINS_H

#include <algorithm>
#include <array>
#include <compare> // Used for std::strong_ordering.
#include <cstddef> // Used for std::size_t type.
#include <deque>
#include <initializer_list>
#include <list>
#include <ostream>
#include <string>
#include <string_view>
#include <utility> // Used for std::move and std::exchange functions.

enum class WagonType { FIRST_CLASS,
                       SECOND_CLASS,
                       SLEEPER,
                       DINING };

namespace trains_details {
// Numbers of seats in the standard series.
inline constexpr std::array<std::size_t, 4> STANDARD_SEATS{
    54, 80, 30, 32};
// Names of the wagon types used when printing. The order of the elements is
// the same as the order of the values of WagonType.
inline constexpr std::array<std::string_view, 4> TYPE_NAMES{
    "1 klasa", "2 klasa", "sypialny", "restauracyjny"};

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
        : type_{type}, seats_{seats} {
    }

    // Creates the standard series of the given type.
    constexpr WagonSeries(WagonType type)
        : WagonSeries{type, trains_details::standard_seats(type)} {
    }

    WagonSeries() = delete;

    constexpr bool operator==(const WagonSeries &) const = default;

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
inline std::ostream &operator<<(std::ostream &os, const WagonSeries &series) {
    return os << '[' << trains_details::type_name(series.type())
              << ", miejsca: " << series.seats() << ']';
}

inline constexpr WagonSeries STANDARD_FIRST_CLASS{WagonType::FIRST_CLASS};
inline constexpr WagonSeries STANDARD_SECOND_CLASS{WagonType::SECOND_CLASS};
inline constexpr WagonSeries STANDARD_SLEEPER{WagonType::SLEEPER};
inline constexpr WagonSeries STANDARD_DINING{WagonType::DINING};

// Wagon with a unique number. The number 0 means that the object is empty,
// i.e. it does not represent any wagon.
class Wagon {
  public:
    using unique_id_t = std::size_t;

    Wagon() = default;

    // Builds a new wagon of the given series with the next number.
    explicit Wagon(const WagonSeries &series)
        : id_{++ever_built_}, series_{series} {
        ++in_service_;
    }

    Wagon(const Wagon &other) {
        *this = other;
    }

    // Moving keeps the number and the series.
    Wagon(Wagon &&other) {
        *this = std::move(other);
    }

    // Rebuilds the wagon, so that it has the series of the other one.
    Wagon &operator=(const Wagon &other) {
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

    Wagon &operator=(Wagon &&other) {
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

    bool operator==(const Wagon &other) const {
        return id_ == other.id_;
    }

    std::strong_ordering operator<=>(const Wagon &other) const {
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

inline std::ostream &operator<<(std::ostream &os, const Wagon &wagon) {
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
        : designation_{designation.empty()
                           ? std::string{trains_details::UNKNOWN_DESIGNATION}
                           : std::move(designation)},
          max_wagons_{max_wagons} {
    }

    // Locomotives cannot be copied.
    Locomotive(const Locomotive &) = delete;
    Locomotive &operator=(const Locomotive &) = delete;

    // Moving leaves the source empty.
    Locomotive(Locomotive &&other) {
        *this = std::move(other);
    }

    // The locomotive represented so far by this object stops existing.
    Locomotive &operator=(Locomotive &&other) {
        if (this != &other) {
            designation_ = std::move(other.designation_);
            max_wagons_ = std::exchange(other.max_wagons_, 0);
            other.designation_.clear();
        }
        return *this;
    }

    const std::string &designation() const {
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

inline std::ostream &operator<<(std::ostream &os, const Locomotive &loco) {
    if (!loco) {
        return os << "brak lokomotywy";
    }
    return os << loco.designation() << " (uciąg: " << loco.max_wagons() << ')';
}

// Train composed of a locomotive and a sequence of wagons attached to it.
// The capacity of the train is the maximal number of wagons its locomotive
// can pull.
class Train {
  public:
    // Creates an empty train: no locomotive and no wagons.
    Train() = default;

    // Attaches copies of the wagons from the list, in the given order, as long
    // as they fit. Empty wagons and wagons that do not fit are skipped, and no
    // copies of them are built.
    explicit Train(Locomotive &&locomotive,
                   std::initializer_list<Wagon> wagons = {})
        : locomotive_{std::move(locomotive)} {
        for (const Wagon &wagon : wagons) {
            *this += wagon;
        }
    }

    Train(const Train &) = delete;
    Train &operator=(const Train &) = delete;

    // The train moved from has no locomotive and no wagons.
    Train(Train &&other)
        : locomotive_{std::move(other.locomotive_)},
          wagons_{std::exchange(other.wagons_, {})} {
    }

    Train &operator=(Train &&other) {
        if (this != &other) {
            locomotive_ = std::move(other.locomotive_);
            wagons_ = std::exchange(other.wagons_, {});
        }
        return *this;
    }

    // Attaches a newly built copy of the wagon, if it can be attached.
    Train &operator+=(const Wagon &wagon) {
        if (can_attach(wagon)) {
            wagons_.push_back(wagon);
        }
        return *this;
    }

    // Attaches the wagon itself, if it can be attached.
    Train &operator+=(Wagon &&wagon) {
        if (can_attach(wagon)) {
            wagons_.push_back(std::move(wagon));
        }
        return *this;
    }

    // Positions are numbered from 0 and must be smaller than size().
    const Wagon &operator[](std::size_t position) const {
        return wagons_[position];
    }

    std::size_t size() const {
        return wagons_.size();
    }

    std::size_t capacity() const {
        return locomotive_.max_wagons();
    }

    // Total number of seats in all the wagons.
    std::size_t seats() const {
        return std::ranges::fold_left(
            wagons_, std::size_t{0},
            [](std::size_t sum, const Wagon &wagon) {
                return sum + wagon.series().seats();
            });
    }

    const Locomotive &locomotive() const {
        return locomotive_;
    }

    // Checks whether the train has a locomotive.
    explicit operator bool() const {
        return static_cast<bool>(locomotive_);
    }

  private:
    bool can_attach(const Wagon &wagon) const {
        return static_cast<bool>(wagon) && size() < capacity();
    }

    Locomotive locomotive_;
    // Unlike std::vector, std::deque never relocates the stored wagons when
    // a new one is appended, so no wagon is ever copied (rebuilt) by accident.
    std::deque<Wagon> wagons_;
};

inline std::ostream &operator<<(std::ostream &os, const Train &train) {
    os << "Skład: " << train.locomotive();
    for (std::size_t position = 0; position < train.size(); ++position) {
        os << "\n\t" << train[position];
    }
    return os;
}

// Depot storing locomotives and wagons in the order of their arrival.
class Depot {
  public:
    Depot() = default;

    Depot(const Depot &) = delete;
    Depot &operator=(const Depot &) = delete;

    // The main depot is stocked in a local object and then moved into place.
    Depot(Depot &&) = default;
    Depot &operator=(Depot &&) = default;

    // The depot takes only existing rolling stock, hence only r-values are
    // accepted. Empty objects are not taken.
    Depot &operator+=(Locomotive &&locomotive) {
        if (locomotive) {
            locomotives_.push_back(std::move(locomotive));
        }
        return *this;
    }

    Depot &operator+=(Wagon &&wagon) {
        if (wagon) {
            wagons_.push_back(std::move(wagon));
        }
        return *this;
    }

    // Releases the first locomotive or an empty one if there is none.
    Locomotive release_locomotive() {
        if (locomotives_.empty()) {
            return Locomotive{};
        }
        Locomotive released{std::move(locomotives_.front())};
        locomotives_.pop_front();
        return released;
    }

    // Releases the first wagon of the series or an empty one if there is none.
    // The remaining wagons keep their order.
    Wagon release_wagon(WagonSeries series) {
        const auto found = std::ranges::find(wagons_, series, &Wagon::series);
        if (found == wagons_.end()) {
            return Wagon{};
        }
        Wagon released{std::move(*found)};
        wagons_.erase(found);
        return released;
    }

    std::size_t locomotives() const {
        return locomotives_.size();
    }

    std::size_t wagons() const {
        return wagons_.size();
    }

    std::size_t wagons(WagonSeries series) const {
        return static_cast<std::size_t>(
            std::ranges::count(wagons_, series, &Wagon::series));
    }

  private:
    std::deque<Locomotive> locomotives_;
    // Wagons are released from the middle, which std::list does without
    // moving the other ones.
    std::list<Wagon> wagons_;
};

namespace trains_details {
// Parameters of a locomotive to be built.
struct LocomotiveSpec {
    std::string_view designation;
    std::size_t max_wagons;
};

// Number of wagons of the standard series of the given type to be built.
struct WagonBatch {
    WagonType type;
    std::size_t count;
};

// Initial rolling stock of the main depot, in the order of arrival.
inline constexpr std::array<LocomotiveSpec, 2> MAIN_DEPOT_LOCOMOTIVES{{
    {"EU07-001", 8},
    {"EP09-020", 12},
}};

inline constexpr std::array<WagonBatch, 4> MAIN_DEPOT_WAGONS{{
    {WagonType::FIRST_CLASS, 1},
    {WagonType::SECOND_CLASS, 4},
    {WagonType::SLEEPER, 2},
    {WagonType::DINING, 1},
}};

// Builds the initial rolling stock of the main depot. The order matters,
// because wagons get their numbers in the order of building.
inline Depot make_main_depot() {
    Depot depot;
    for (const auto &[designation, max_wagons] : MAIN_DEPOT_LOCOMOTIVES) {
        depot += Locomotive{std::string{designation}, max_wagons};
    }
    for (const auto &[type, count] : MAIN_DEPOT_WAGONS) {
        for (std::size_t built = 0; built < count; ++built) {
            depot += Wagon{WagonSeries{type}};
        }
    }
    return depot;
}
} // namespace trains_details

// The main depot is created during the first call, not earlier.
inline Depot &main_depot() {
    static Depot depot{trains_details::make_main_depot()};
    return depot;
}

// Dispatcher assembling trains from the rolling stock of one depot.
// The depot is not owned by the dispatcher.
class Dispatcher {
  public:
    // The default argument is evaluated at the call, so the main depot is
    // not created unless it is needed.
    explicit Dispatcher(Depot &depot = main_depot())
        : depot_{depot} {
    }

    // A dispatcher cannot serve a temporary depot.
    Dispatcher(const Depot &&) = delete;

    Dispatcher(const Dispatcher &) = default;
    Dispatcher &operator=(const Dispatcher &) = delete;

    const Depot &yard() const {
        return depot_;
    }

    // Takes the first locomotive and then the wagons of the series, in the
    // order of their arrival, until the train has the requested number of
    // wagons, is full or there are no more wagons of the series.
    // The method is const, because the depot is not a part of the dispatcher.
    Train assemble(WagonSeries series, std::size_t count) const {
        if (depot_.locomotives() == 0) {
            return Train{};
        }
        Train train(depot_.release_locomotive());
        const std::size_t target = std::min(count, train.capacity());
        while (train.size() < target && depot_.wagons(series) > 0) {
            train += depot_.release_wagon(series);
        }
        return train;
    }

  private:
    Depot &depot_;
};

#endif // TRAINS_TRAINS_H
