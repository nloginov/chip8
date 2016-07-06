#pragma once

#include <utils/bitutils.h>

#include <boost/variant.hpp>

struct TEmpty {
    static TEmpty Parse(uint16_t opcode) {
        return TEmpty {};
    }
};

struct TAddress {
    uint16_t Value;
    static TAddress Parse(uint16_t opcode) {
        return TAddress { .Value = GetOctetsRange<1,3>(opcode) };
    }

};

struct TVar {
    uint8_t X;

    static TVar Parse(uint16_t opcode) {
        return TVar { .X = GetOctetAt<3>(opcode) };
    }
};

struct TVarWithConst {
    uint8_t X;
    uint8_t Const;

    static TVarWithConst Parse(uint16_t opcode) {
        return TVarWithConst { .X = GetOctetAt<3>(opcode), .Const = GetOctetsRange<1,2>(opcode) };
    }
};

struct TTwoVarsWithConst {
    uint8_t X;
    uint8_t Y;
    uint8_t Const;

    static TTwoVarsWithConst Parse(uint16_t opcode) {
        return TTwoVarsWithConst { .X = GetOctetAt<3>(opcode), .Y = GetOctetAt<2>(opcode), .Const = GetOctetAt<1>(opcode) };
    }
};

struct TTwoVars {
    uint8_t X;
    uint8_t Y;

    static TTwoVars Parse(uint16_t opcode) {
        return TTwoVars { .X = GetOctetAt<3>(opcode), .Y = GetOctetAt<2>(opcode) };
    }
};

enum class EOperationType {
    CLS,
    RET,
    JUMP,
    CALL,
    SE_CONST,
    SE_KEY,
    SNE_KEY,
    SNE_CONST,
    SE_VAR,
    LD_CONST,
    ADD_CONST,
    LD_VAR,
    OR_VAR,
    AND_VAR,
    XOR_VAR,
    ADD_VAR,
    SUB_VAR,
    SHR_VAR,
    SUBN_VAR,
    SHL_VAR,
    SNE_VAR,
    LD_ADDR,
    RND,
    DRAW,
    LD_ST,
    LD_DT,
    LD_KEY,
    ADD_ADDR,
    LD_MEM,
    STORE_DT,
    STORE_ST,
    LD_SPRITE,
    STORE_MEM,
    STORE_BCD_VAR,
};

class TOpcode {
public:
    using TArguments = boost::variant<TEmpty, TAddress, TVar, TVarWithConst, TTwoVars, TTwoVarsWithConst>;

public:
    TOpcode(EOperationType opType, const TArguments& arguments)
            : OperationType(opType)
            , Arguments(arguments)
    {}

    EOperationType GetOperationType() const {
        return OperationType;
    };

    template <typename T>
    const T& GetArgs() const {
        return boost::get<T>(Arguments);
    }

private:
    EOperationType OperationType;
    TArguments Arguments;
};
