#ifndef EB1637D2_91DD_44C8_994E_CEACB924F9DC
#define EB1637D2_91DD_44C8_994E_CEACB924F9DC

#include <random>
#include <instrument.hpp>

namespace mc
{
    // The Pricer class is responsible for pricing financial instruments
    // using Monte Carlo simulation data.
    // It implements the Visitor pattern to handle different types of instruments.
    class Pricer : public Visitor
    {
    public:
        Pricer(const SimulationData &data) : data_(data) {};

        void reset() { npv_ = 0.0; }

        // simple vanilla npv calculation
        void operator()(const EuropeanOption &inst) override
        {
            double npv = 0.0;
            for (const auto &path : data_)
            {
                double payoff = inst.payoff(path.get_spot(inst.maturity()));
                double df = path.get_df(inst.maturity()); // actually this is a non-random number, but we could have a model with stochastic df
                npv += df * payoff;
            }
            npv /= data_.size();
            npv_ += npv;
        };

        void operator()(const KnockAndOutOption &inst) override
        {
            // full path‐dependent barrier payoff
            double npv = 0.0;
            for (const auto &path : data_)
            {
                const auto &spots = path.get_spots();
                bool knocked = std::any_of(spots.begin(), spots.end(),
                                           [&](double s)
                                           { return s <= inst.barrier(); });

                // if the path is not knocked out, calculate the payoff
                if (!knocked)
                {
                    double payoff = inst.payoff(path.get_spot(inst.maturity()));
                    npv += path.get_df(inst.maturity()) * payoff;
                }
            }
            npv /= data_.size();
            npv_ += npv;
        };

        double npv() const { return npv_; };

    private:
        std::atomic<double> npv_ = 0.0; // atomic to avoid race conditions
        const SimulationData &data_;
    };
} // namespace mc

#endif /* EB1637D2_91DD_44C8_994E_CEACB924F9DC */
