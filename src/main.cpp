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
    instruments.push_back(EuropeanOption(100.0, 360, OptionType::PUT)); // max(spot-strike, 0) // E[max(spot_T-strike, 0)]*exp(-rT)
    instruments.push_back(KnockAndOutOption(90.0, 100.0, 720));         // max(spot-strike, 0) // E[ if spot_i < barrera : max(spot_T-strike, 0), 0]*exp(-rT)
    return instruments;
}

int main()
{

    auto start = std::chrono::high_resolution_clock::now();

    SimulationData data = BlackScholesModel(0.05, 0.2, 100)
                              .with_seed(1234)
                              .with_steps(720)
                              .with_dt(1.0 / 360.0)
                              .with_mt(true)
                              .with_simulations(100'000)
                              .simulate();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Simulation took " << duration << " ms" << std::endl;

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