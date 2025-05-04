#ifndef D1BBE9E4_8CA1_40CD_9DA7_17C83FC62403
#define D1BBE9E4_8CA1_40CD_9DA7_17C83FC62403

#include <vector>
#include <config.hpp>

namespace mc
{
    class PathData;
    using SimulationData = std::vector<PathData>;

    class PathData
    {
    public:
        PathData() = default;
        PathData(const std::vector<double> &df, const std::vector<double> &spot, const std::vector<Date> &time)
            : df_(df), spot_(spot), time_(time) {}
        // rvalue reference constructor, moves the data from the other object
        // to the new object. This is useful for transferring ownership of
        // resources without copying them.
        PathData(PathData &&other) noexcept
        {
            df_ = std::move(other.df_);
            spot_ = std::move(other.spot_);
            time_ = std::move(other.time_);
        };

        PathData &operator=(PathData &&other) noexcept
        {
            if (this != &other)
            {
                df_ = std::move(other.df_);
                spot_ = std::move(other.spot_);
                time_ = std::move(other.time_);
            }
            return *this;
        };

        // Copy constructor
        PathData(const PathData &other) = default;
        size_t size() const { return df_.size(); }

        // Reserve space for the vectors to avoid multiple allocations.
        void reserve(size_t size)
        {
            df_.reserve(size);
            spot_.reserve(size);
            time_.reserve(size);
        }

        void push_back(double df_value, double spot_value, Date time_value)
        {
            df_.push_back(df_value);
            spot_.push_back(spot_value);
            time_.push_back(time_value);
        }

        double get_spot(Date t) const
        {
            if (t < spot_.size())
            {
                return spot_[t];
            }
            throw std::out_of_range("Index out of range");
        }

        double get_df(Date t) const
        {
            if (t < df_.size())
            {
                return df_[t];
            }
            throw std::out_of_range("Index out of range");
        }

        const std::vector<double> &get_spots() const { return spot_; }
        const std::vector<double> &get_dfs() const { return df_; }
        const std::vector<Date> &get_times() const { return time_; }

    private:
        std::vector<double> df_;   // exp(-rT)
        std::vector<double> spot_; // S+ds
        std::vector<Date> time_;   // 1,2,3,4...
    };
} // namespace mc

#endif /* D1BBE9E4_8CA1_40CD_9DA7_17C83FC62403 */
