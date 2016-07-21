#include <gtest/gtest.h>

#define private public
#include <chip8.h>
#include <opcode/parser.h>

class TestOpcodes : public ::testing::Test {

protected:
    TestOpcodes()
     : Cpu(State) {};


    virtual void SetUp() {

    }

    TChip8Machine::TState State;
    TChip8Machine::TCPU Cpu;
};

TEST_F(TestOpcodes, TestRET) {
    State.PC = 0;
    State.Stack.push(42);
    State.Stack.push(43);

    Cpu.Return(TOpcode(EOperationType::RET, TEmpty{}));

    ASSERT_EQ(43, State.PC);
    ASSERT_EQ(1, State.Stack.size());
    ASSERT_EQ(42, State.Stack.top());
}

TEST_F(TestOpcodes, TestCALL) {
    State.PC = 42;

    ASSERT_TRUE(State.Stack.empty());
    Cpu.Call(TOpcode(EOperationType::CALL, TAddress{.Value = 0x123}));

    ASSERT_EQ(0x123, State.PC);
    ASSERT_EQ(1, State.Stack.size());
    ASSERT_EQ(42, State.Stack.top());
}

TEST_F(TestOpcodes, TestADDI) {
    State.I = 0;
    State.V.at(1) = 42;
    State.V.at(2) = 10;
    Cpu.AddWithAddr(TOpcode(EOperationType::ADD_ADDR, TVar {.X = 1}));
    ASSERT_EQ(42, State.I);
    Cpu.AddWithAddr(TOpcode(EOperationType::ADD_ADDR, TVar {.X = 2}));
    ASSERT_EQ(52, State.I);

    ASSERT_EQ(42, State.V.at(1));
    ASSERT_EQ(10, State.V.at(2));
}

TEST_F(TestOpcodes, TestADDCONST) {
    State.V.at(1) = 10;
    Cpu.AddConst(TOpcode(EOperationType::ADD_CONST, TVarWithConst {.X = 1, .Const = 42}));
    ASSERT_EQ(52, State.V.at(1));

    State.V.at(1) = 200;
    Cpu.AddConst(TOpcode(EOperationType::ADD_CONST, TVarWithConst {.X = 1, .Const = 100}));
    ASSERT_EQ(44, State.V.at(1));

    State.V.at(1) = 5;
    Cpu.AddConst(TOpcode(EOperationType::ADD_CONST, TVarWithConst {.X = 1, .Const = 0xFF}));
    ASSERT_EQ(4, State.V.at(1));
}

TEST_F(TestOpcodes, TestADDVAR) {
    State.V.at(1) = 10;
    State.V.at(2) = 5;
    State.V.at(0xF) = 0;
    Cpu.AddWithVar(TOpcode(EOperationType::ADD_VAR, TTwoVars {.X = 1, .Y = 2}));
    ASSERT_EQ(15, State.V.at(1));
    ASSERT_EQ(5, State.V.at(2));
    ASSERT_EQ(0, State.V.at(0xF));

    State.V.at(1) = 10;
    State.V.at(2) = 5;
    State.V.at(0xF) = 1;
    Cpu.AddWithVar(TOpcode(EOperationType::ADD_VAR, TTwoVars {.X = 2, .Y = 1}));
    ASSERT_EQ(10, State.V.at(1));
    ASSERT_EQ(15, State.V.at(2));
    ASSERT_EQ(0, State.V.at(0xF));


    State.V.at(1) = 10;
    State.V.at(2) = 250;
    State.V.at(0xF) = 0;
    Cpu.AddWithVar(TOpcode(EOperationType::ADD_VAR, TTwoVars {.X = 2, .Y = 1}));
    ASSERT_EQ(10, State.V.at(1));
    ASSERT_EQ(4, State.V.at(2));
    ASSERT_EQ(1, State.V.at(0xF));

    State.V.at(1) = 10;
    State.V.at(2) = 250;
    State.V.at(0xF) = 1;
    Cpu.AddWithVar(TOpcode(EOperationType::ADD_VAR, TTwoVars {.X = 1, .Y = 2}));
    ASSERT_EQ(4, State.V.at(1));
    ASSERT_EQ(250, State.V.at(2));
    ASSERT_EQ(1, State.V.at(0xF));
}


TEST_F(TestOpcodes, TestSEVAR) {
    State.PC = 0;
    State.V.at(1) = 2;
    State.V.at(2) = 2;
    Cpu.SkipIfEqualToVar(TOpcode(EOperationType::SE_VAR, TTwoVars {.X = 1, .Y = 2}));
    ASSERT_EQ(2, State.PC);
    ASSERT_EQ(2, State.V.at(1));
    ASSERT_EQ(2, State.V.at(2));

    State.PC = 2;
    State.V.at(1) = 2;
    State.V.at(2) = 3;
    Cpu.SkipIfEqualToVar(TOpcode(EOperationType::SE_VAR, TTwoVars {.X = 1, .Y = 2}));
    ASSERT_EQ(2, State.PC);
    ASSERT_EQ(2, State.V.at(1));
    ASSERT_EQ(3, State.V.at(2));
}