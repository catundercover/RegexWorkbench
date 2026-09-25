// Declares the platform-specific operation-runner factory hook.
#pragma once

#include <memory>

namespace app::operations
{
    class OperationRunner;
}

namespace app::operations::detail
{
    // Implemented by exactly one platform-specific translation unit.
    [[nodiscard]] std::unique_ptr<OperationRunner> make_platform_runner();
}
