#ifndef ACB10E74_19E3_4866_8E8C_AE8C4F70DDAB
#define ACB10E74_19E3_4866_8E8C_AE8C4F70DDAB

#include <config.hpp>
#include <cmath>
#include <visitor.hpp>

namespace mc
{

    enum OptionType
    {
        CALL,
        PUT
    };

    // Vanilla European option
    // with a strike price and maturity date.
    class EuropeanOption
    {
    public:
        EuropeanOption(double strike, Date maturity, OptionType type = OptionType::CALL) : strike_(strike), maturity_(maturity), type_(type) {};

        Date maturity() const
        {
            return maturity_;
        }

        double payoff(double s) const
        {
            return type_ == OptionType::CALL ? std::max(s - strike_, 0.0) : std::max(strike_ - s, 0.0);
        }

    private:
        double strike_;
        Date maturity_;
        OptionType type_;
    };

    // Knock-in and knock-out options
    // with a barrier level, strike price, and maturity date.
    // The barrier level is the price level that, if breached, will knock the option in or out.
    class KnockAndOutOption
    {
    public:
        KnockAndOutOption(double barrier, double strike, Date maturity, OptionType type = OptionType::CALL) : barrier_(barrier), strike_(strike), maturity_(maturity), type_(type) {};

        Date maturity() const { return maturity_; }

        double barrier() const { return barrier_; }

        double payoff(double s) const
        {
            return type_ == OptionType::CALL ? std::max(s - strike_, 0.0) : std::max(strike_ - s, 0.0);
        }

    private:
        double strike_;
        Date maturity_;
        OptionType type_;
        double barrier_;
    };

}

#endif /* ACB10E74_19E3_4866_8E8C_AE8C4F70DDAB */
