#ifndef E9258E57_03E3_42BE_B834_32FD67ED7AC4
#define E9258E57_03E3_42BE_B834_32FD67ED7AC4

#include <variant>

namespace mc
{

    class EuropeanOption;
    class KnockAndOutOption;

    struct Visitor
    {
        // Processes a KnockAndOutOption instance.
        virtual void operator()(const KnockAndOutOption &inst) = 0;
        // This method is called when visiting a EuropeanOption instance.
        virtual void operator()(const EuropeanOption &inst) = 0;
        // Virtual destructor to ensure proper cleanup of derived classes.
        virtual ~Visitor() = default;
    };

    // Define a variant type that can hold either EuropeanOption or KnockAndOutOption.
    using Instrument = std::variant<EuropeanOption, KnockAndOutOption>;
};

#endif /* E9258E57_03E3_42BE_B834_32FD67ED7AC4 */
