#include <model.hpp>
#include <instrument.hpp>
#include <pricer.hpp>
#include <iostream>
#include <threadpool.hpp>
#include <algorithm>

using namespace mc;

ThreadPool ThreadPool::myInstance;
thread_local size_t ThreadPool::myTLSNum = 0;

auto create_portfolio()
{
    std::vector<Instrument> instruments;
    instruments.push_back(EuropeanOption(100, 360, OptionType::PUT)); // max(spot-strike, 0) // E[max(spot_T-strike, 0)]*exp(-rT)
    instruments.push_back(KnockAndOutOption(90, 100, 720));           // max(spot-strike, 0) // E[ if spot_i < barrera : max(spot_T-strike, 0), 0]*exp(-rT)
    return instruments;
}

int main()
{

    SimulationData data = BlackScholesModel(0.05, 0.2, 100)
                              .with_seed(1234)
                              .with_steps(720)
                              .with_dt(1.0 / 360)
                              .with_simulations(1'000'000) // python: 1_000_000
                              .simulate(); // geometric brownian motion

    // Visitor pattern example
    Pricer pricer(data);
    std::vector<Instrument> portfolio = create_portfolio();

    std::for_each(portfolio.begin(), portfolio.end(), [&pricer](const auto &inst)
                  { std::visit(pricer, inst); });

    // we could do (windows), which allows for Execution Policy
    // std::for_each(std::execution::par, portfolio.begin(), portfolio.end(), [&pricer](const auto &inst)
    //               { std::visit(pricer, inst); });

    // since we are only reading the data and updating an atomic
    // variable (npv_), we don't need to lock the data -> the pricer is thread safe.

    double portfolio_npv = pricer.npv();
    std::cout << "Portfolio NPV: " << portfolio_npv << std::endl;
    return 0;
}