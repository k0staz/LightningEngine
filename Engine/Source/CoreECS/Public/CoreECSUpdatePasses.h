#pragma once
#include "Multithreading/UpdatePasses.h"

namespace LE
{
REGISTER_UPDATE_PASS(TestUpdatePass, Color::Green())
REGISTER_UPDATE_PASS(RenderPass, Color::Red(),TestUpdatePass)
}
