#ifndef AC1CD054_CDCC_489E_A3CD_41138FE1DC4B
#define AC1CD054_CDCC_489E_A3CD_41138FE1DC4B

#include <data.hpp>
#include <random>
#include <threadpool.hpp>
#include <iostream>

namespace mc
{
    // The idea is that a model can be configured with a number of parameters
    template <typename T>
    // CRTP: Curiously Recurring Template Pattern
    class ConfigurableModel
    {
    public:
        T &with_steps(size_t n)
        {
            steps_ = n + 1;
            return self();
        }
        T &with_simulations(size_t n)
        {
            simulations_ = n;
            return self();
        }
        T &with_dt(double dt)
        {
            dt_ = dt;
            return self();
        }
        T &with_seed(uint32_t s)
        {
            seed_ = s;
            return self();
        }

        T &with_mt(bool mt)
        {
            mt_ = mt;
            return self();
        }

        virtual SimulationData simulate() = 0;

    protected:
        double draw_standard_normal()
        {
            static thread_local std::mt19937 rng_{seed_};
            static thread_local std::normal_distribution<double> N01{0.0, 1.0};
            return N01(rng_);
        }

        size_t steps_ = 360;
        size_t simulations_ = 1000;
        double dt_ = 1.0 / 252.0;
        uint32_t seed_ = 5489u;
        bool mt_ = false; // multithreaded

    private:
        // we need a way to return the derived class in the base class methods
        T &self() { return static_cast<T &>(*this); }
    };

    class BlackScholesModel final : public ConfigurableModel<BlackScholesModel>
    {
    public:
        BlackScholesModel(double r, double sigma, double s0, double q = 0.0)
            : r_(r), sigma_(sigma), s0_(s0), q_(q) {}

        BlackScholesModel &with_sigma(double v)
        {
            sigma_ = v;
            return *this;
        }
        BlackScholesModel &with_s0(double v)
        {
            s0_ = v;
            return *this;
        }
        BlackScholesModel &with_r(double v)
        {
            r_ = v;
            return *this;
        }
        BlackScholesModel &with_q(double v)
        {
            q_ = v;
            return *this;
        }

        // The simulate function is the main function that runs the simulation
        // It uses the ThreadPool to spawn tasks for each simulation. Other ways
        // to do this would be to use OpenMP, std::for_each, but the thread pool
        // approach is OS independent.
        SimulationData simulate() override
        {
            SimulationData sims;
            sims.resize(simulations_);
            ThreadPool *pool = ThreadPool::getInstance();

            mt_ ? pool->start(std::thread::hardware_concurrency() / 2)
                : pool->start(0);

            std::vector<TaskHandle> futures(simulations_);
            for (size_t i = 0; i < simulations_; ++i)
            {
                auto task = [&, i]()
                {
                    PathData &path = sims[i];
                    path.reserve(steps_);
                    const double drift = (r_ - q_) - 0.5 * sigma_ * sigma_;
                    const double vol_dt = sigma_ * std::sqrt(dt_);
                    double spot = s0_;
                    path.push_back(1.0, spot, 1);
                    for (size_t j = 1; j < steps_; ++j)
                    {
                        const double t = j * dt_;
                        const double df = std::exp(-r_ * t);
                        const double w = draw_standard_normal();

                        spot *= std::exp(drift * dt_ + vol_dt * w);
                        path.push_back(df, spot, j + 1);
                    }
                    return true;
                };
                futures[i] = pool->spawnTask(task);
            };
            for (size_t i = 0; i < simulations_; ++i)
            {
                pool->activeWait(futures[i]);
            }
            return sims; // rvo
        }

    private:
        double sigma_,
            s0_,
            r_,
            q_;
    };

} // namespace mc

#endif /* AC1CD054_CDCC_489E_A3CD_41138FE1DC4B */
