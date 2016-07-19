#include <map>

#include "parser.h"
#include <utils/bitutils.h>
#include <ios>
#include <sstream>

namespace {
    template<typename TArgType>
    std::function<TOpcode(uint16_t)> RegisterOpcode(EOperationType operationType) {
        return [operationType](uint16_t opcode) {
            return TOpcode(operationType, TArgType::Parse(opcode));
        };
    }

    template<size_t NCount>
    std::function<TOpcode(uint16_t)> First(std::initializer_list<std::pair<uint16_t, std::function<TOpcode(uint16_t)>>> pairs) {
        std::map <uint16_t, std::function<TOpcode(uint16_t)>> lastMap;
        for (const auto& pair : pairs) {
            lastMap.insert(pair);
        }
         return [lastMap](uint16_t opcode) {
            auto& func = lastMap.at(GetOctetsRange<1, NCount>(opcode));
            return func(opcode);
        };
    }



    const std::map<uint8_t, std::function<TOpcode(uint16_t)>> Instructions = {
            {0x0, First<3>({
                                   {0x0E0, RegisterOpcode<TEmpty>(EOperationType::CLS)},
                                   {0x0EE, RegisterOpcode<TEmpty>(EOperationType::RET)}
                           })},
            {0x1, RegisterOpcode<TAddress>(EOperationType::JUMP)},
            {0x2, RegisterOpcode<TAddress>(EOperationType::CALL)},
            {0x3, RegisterOpcode<TVarWithConst>(EOperationType::SE_CONST)},
            {0x4, RegisterOpcode<TVarWithConst>(EOperationType::SNE_CONST)},
            {0x5, RegisterOpcode<TTwoVars>(EOperationType::SE_VAR)},
            {0x6, RegisterOpcode<TVarWithConst>(EOperationType::LD_CONST)},
            {0x7, RegisterOpcode<TVarWithConst>(EOperationType::ADD_CONST)},
            {0x8, First<1>({
                                   {0x0, RegisterOpcode<TTwoVars>(EOperationType::LD_VAR)},
                                   {0x1, RegisterOpcode<TTwoVars>(EOperationType::OR_VAR)},
                                   {0x2, RegisterOpcode<TTwoVars>(EOperationType::AND_VAR)},
                                   {0x3, RegisterOpcode<TTwoVars>(EOperationType::XOR_VAR)},
                                   {0x4, RegisterOpcode<TTwoVars>(EOperationType::ADD_VAR)},
                                   {0x5, RegisterOpcode<TTwoVars>(EOperationType::SUB_VAR)},
                                   {0x6, RegisterOpcode<TTwoVars>(EOperationType::SHR_VAR)},
                                   {0x7, RegisterOpcode<TTwoVars>(EOperationType::SUBN_VAR)},
                                   {0xE, RegisterOpcode<TTwoVars>(EOperationType::SHL_VAR)}
                           })},
            {0x9, RegisterOpcode<TTwoVars>(EOperationType::SNE_VAR)},
            {0xA, RegisterOpcode<TAddress>(EOperationType::LD_ADDR)},
            {0xC, RegisterOpcode<TVarWithConst>(EOperationType::RND)},
            {0xD, RegisterOpcode<TTwoVarsWithConst>(EOperationType::DRAW)},
            {0xE, First<2>({
                                   {0x9E, RegisterOpcode<TVar>(EOperationType::SE_KEY)},
                                   {0xA1, RegisterOpcode<TVar>(EOperationType::SNE_KEY)}
                           })},
            {0xF, First<2>({
                                   {0x0A, RegisterOpcode<TVar>(EOperationType::LD_KEY)},
                                   {0x07, RegisterOpcode<TVar>(EOperationType::LD_DT)},
                                   {0x1E, RegisterOpcode<TVar>(EOperationType::ADD_ADDR)},
                                   {0x15, RegisterOpcode<TVar>(EOperationType::STORE_DT)},
                                   {0x18, RegisterOpcode<TVar>(EOperationType::STORE_ST)},
                                   {0x29, RegisterOpcode<TVar>(EOperationType::LD_SPRITE)},
                                   {0x33, RegisterOpcode<TVar>(EOperationType::STORE_BCD_VAR)},
                                   {0x55, RegisterOpcode<TVar>(EOperationType::STORE_MEM)},
                                   {0x65, RegisterOpcode<TVar>(EOperationType::LD_MEM)},
                           })},
    };
}

const TOpcode TOpcodeParser::Parse(uint16_t opcode) {
    const uint16_t lastOctet = GetLastOctet(opcode);
    try {
        const auto& function = Instructions.at(lastOctet);
        return function(opcode);
    }
    catch (...) {
        std::stringstream ss;
        ss << "Unknown opcode: " << std::hex << opcode;
        throw std::logic_error(ss.str());
    }
}
