// Declares synchronous execution of validated application operations.
#pragma once

#include "app/operations/OperationModels.hpp"

namespace app::operations
{
    // Executes one complete operation, converting all failures into a typed result.
    [[nodiscard]] OperationResult execute(OperationRequest request);
}
