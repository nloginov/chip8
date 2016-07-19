#pragma once

#include "types.h"

class TOpcodeParser
{
    public:
        static const TOpcode Parse(uint16_t opcode);
};
