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

        // rvalue reference constructor, moves the data from the other object
        // to the new object. This is useful for transferring ownership of
        // resources without copying them.
        PathData(PathData &&other) noexcept
            : df(std::move(other.df)), spot(std::move(other.spot)), time(std::move(other.time)) {}

        size_t size() const { return df.size(); }

        // Reserve space for the vectors to avoid multiple allocations.
        void reserve(size_t size)
        {
            df.reserve(size);
            spot.reserve(size);
            time.reserve(size);
        }

        void push_back(double df_value, double spot_value, Date time_value)
        {
            df.push_back(df_value);
            spot.push_back(spot_value);
            time.push_back(time_value);
        }

        double get_spot(Date t) const
        {
            if (t < spot.size())
            {
                return spot[t];
            }
            throw std::out_of_range("Index out of range");
        }

        double get_df(Date t) const
        {
            if (t < df.size())
            {
                return df[t];
            }
            throw std::out_of_range("Index out of range");
        }

        const std::vector<double> &get_spots() const { return spot; }
        const std::vector<double> &get_dfs() const { return df; }
        const std::vector<Date> &get_times() const { return time; }

    private:
        std::vector<double> df; // exp(-rT)
        std::vector<double> spot; // S+ds
        std::vector<Date> time; // 1,2,3,4...
    };
} // namespace mc

#endif /* D1BBE9E4_8CA1_40CD_9DA7_17C83FC62403 */
