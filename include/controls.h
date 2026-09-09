#pragma once

namespace Controls
{
void initialize();

// Returns true for each debounced button/click press or completed encoder detent.
bool pollActivity();
} // namespace Controls
