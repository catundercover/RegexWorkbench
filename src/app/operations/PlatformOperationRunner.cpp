// Implements the public factory for the selected platform operation runner.
#include "app/operations/OperationRunner.hpp"
#include "app/operations/detail/PlatformRunnerFactory.hpp"

namespace app::operations
{
    std::unique_ptr<OperationRunner> make_platform_operation_runner()
    {
        return detail::make_platform_runner();
    }
}
