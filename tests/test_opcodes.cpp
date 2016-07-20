#include <gtest/gtest.h>

#define private public
#include <chip8.h>
#include <opcode/parser.h>


TEST(TestOpcodes, TestRet) {
    TChip8Machine::TState state;
    state.PC = 0;
    state.Stack.push(42);
    state.Stack.push(43);

    TChip8Machine::TCPU cpu(state);
    cpu.Return(TOpcodeParser::Parse(0x00EE));

    ASSERT_EQ(43, state.PC);
    ASSERT_EQ(1, state.Stack.size());
    ASSERT_EQ(42, state.Stack.top());
}