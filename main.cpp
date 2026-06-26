#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <cstring>
#include <cstdio> // for sscanf

using namespace std;

enum class Opcode
{ 
    ADD, SUB, MUL, DIV, INC, DEC, // Arithmetic
    INPUT, DISPLAY, // IO
    MOV, LOAD, STORE, // DataMovement
    RESET, // Status
    PUSH, POP, // Stack
    SHL, SHR, ROL, ROR // Shift
};

// LIM temp functions to test flag func
enum class Flags : int
{
    OF = 0,
    UF = 1,
    CF = 2, 
    ZF = 3
};
// END

// Base class of register (Encapsulates an 8-bit signed value and flag update logic) 
class Register
{
protected:
    int8_t value; // value of the 8-bit itself

public:
    Register() : value()
    {

    }

    void setValue(int8_t val)
    {
        value = val;
    }

    int8_t getValue() const
    {
        return value;
    }
};

// comment
// implementation of the register movement
class GeneralRegister : public Register
{
    

};

// registers boolean values based on CF OF UF ZF flags
class FlagRegister 
{
private:
    bool cf; // Carry Flag
    bool zf; // Zero Flag
    bool uf; // Underflow Flag
    bool of; // Overflow Flag

public:
    FlagRegister() 
    {
        cf = false;
        zf = false;
        uf = false;
        of = false;
    }

    // Getters for display formatting
    bool getCF() const { return cf; }
    bool getZF() const { return zf; }
    bool getUF() const { return uf; }
    bool getOF() const { return of; }

    // Manual manual reset instruction logic
    void resetFlag(string flagName) 
    {
        if (flagName == "CF") cf = false;
        else if (flagName == "ZF") zf = false;
        else if (flagName == "UF") uf = false;
        else if (flagName == "OF") of = false;
    }

    // Core validation method to check arithmetic bounds
    void updateFlags(int result, bool isArithmetic = false, bool logicalCarry = false) 
    {
        // 1. Zero Flag (ZF): Set when the result of an operation is zero
        zf = (result == 0);

        // 2. Overflow Flag (OF): Result greater than 127
        of = (result > 127);

        // 3. Underflow Flag (UF): Result smaller than -128
        uf = (result < -128);

        // 4. Carry Flag (CF): Set if calculated result of math instructions exceeds 8-bit capacity
        if (isArithmetic) 
        {
            cf = logicalCarry; 
        }
    }
};

// Handles storage and addressing logic over a vector of bytes
class Memory
{
private:
    vector<int8_t> storage;

public:
    Memory(size_t size = 64) : storage(size,0)
    {

    }

    void write(size_t address, int8_t value)
    {
        if (address < storage.size())
        {
            storage[address] = value;
        }
        else
        {
            cerr << "Memory Access Violation: Write at " << address << endl;
        }
    }

    int8_t read(size_t address) const
    {
        if (address < storage.size())
        {
            return storage[address];
        }

        cerr << "Memory Access Violation: Read at " << address << endl;
        return 0;
    }
    //insert code here
};

// Contains registers, memory, PC, executes instructions 
class CPU
{
private:
    Memory memory;          
    FlagRegister* flags;    
    GeneralRegister registers[8];
    int pc;

    // Your Assigned Additions to align with requirements:
    int8_t stackStorage[8]; // The 8-byte system stack managed internally
    int8_t si;              // Stack Index (SI) register starting at 0

public:

    // LIM temp function for testing instruction
    class
    {
    public:
        int8_t data[8] = {};
        int SI = 0;
    } testStack;
    int8_t mem[64] = {};
    int8_t reg[8] = {};
    bool testFlags[4] = {};
    int8_t getReg(int n) const { return reg[n]; }
    void setReg(int n, int8_t val) { reg[n] = val; }
    int8_t getMem(int n) const { return mem[n]; }
    void setMem(int n, int8_t val) { mem[n] = val; }
    void setFlag(Flags f, bool b) 
    {
        testFlags[static_cast<int>(f)] = b;
    }
    void pushStack(int8_t value) 
    {
        testStack.data[getSI()] = value;
        setSI(1);
    }
    int8_t popStack() 
    {
        setSI(-1);
        return testStack.data[getSI()];
    }
    int getSI() const{ return testStack.SI;}
    void setSI(int n) { testStack.SI+=n; };

    void dump(int inst) const 
    {
        cout << "inst" << inst << endl;
        cout << "reg#";
        for (int i = 0; i < 8; i++) cout << static_cast<int>(getReg(i)) << "#";
        cout << "\nflag#";
        for (int i = 0; i < 4; i++) cout << static_cast<int>(testFlags[i]) << "#";
        cout << "\nmem\n";
        for (int i = 0; i < 64; i++) 
        {
            cout << static_cast<int>(mem[i]) << "#";
            if (!((i+1)%8)) cout << endl;
        }
        cout << "stack (not in output format)\n";
        for (int i = 0; i < getSI(); i++) 
        {
            cout << static_cast<int>(testStack.data[i]) << "#";
        }
        if (!getSI()) cout << "no stack data";
        cout << endl << endl;
    }
    // END
};

// Loads programs, decodes instructions, delegates execution to `CPU`
struct ParsedInstruction
{
    char command[20];
    char operand1[20];
    char operand2[20];
};

class Runner
{
private:
    CPU* cpu;
    ParsedInstruction program[100];
    int instructionCount;

public:
    Runner()
    {
        cpu = NULL;
        instructionCount = 0;
    }

    Runner(CPU* c)
    {
        cpu = c;
        instructionCount = 0;
    }

    void setCPU(CPU* c) {
        cpu = c;
    }

    void cleanLine(char line[]) {
        
    }

    void parseLine(char line[], ParsedInstruction& inst) {
        
    }

    bool loadProgram(const char fileName[]) {
        return true;
    }

    void displayInstruction(ParsedInstruction inst) {
        
    }

    void run() {
        
    }

    int getInstructionCount() {
        return instructionCount;
    }
};

// all assembly commands logic go here e.g 'ADD' or 'MOV' (abstract base class)
class Instruction
{
protected:
    CPU& cpu;
    Opcode opcode;

    void updateReg(int opr1, int8_t result)
    {
        // updateFlag(Flags::CF, true);
        cpu.setReg(opr1, result);
    }

    void updateFlag(Flags f, bool val)
    { cpu.setFlag(f, val); }

public:
    Instruction(CPU& c, Opcode opc) : cpu(c) { opcode = opc; }
    virtual ~Instruction() {} // destructor
    virtual void execute() = 0; // pure virtual: causes classes to implement.
};

// Handles 'ADD', 'SUB' and other arithmetic instructions for assembly
class ArithmeticInstruction : public Instruction
{   
private:
    int operand1;
    int8_t operand2;
    bool isReg;
public:
    ArithmeticInstruction(CPU& c, Opcode opc, int opr1, int8_t opr2=1, bool isR=false) : Instruction(c, opc) 
    {
        operand1 = opr1; 
        operand2 = opr2;
        isReg = isR;
    }

    void execute() override
    {
        int8_t result;
        int8_t value1 = cpu.getReg(operand1);
        int8_t value2 = isReg? cpu.getReg(operand2) : operand2; // get value from register[opr2] if is reg  

        switch (opcode)
        {
            case (Opcode::ADD) :
            case (Opcode::INC) :
                result = value1 + value2; // ADD A, B (A = A+B); INC A (A = A+1)
                break;
            case (Opcode::SUB) :
            case (Opcode::DEC) :
                result = value1 - value2; // SUB A, B (A = A-B); DEC A (A = A-1)
                break;
            case (Opcode::MUL) :
                result = value1 * value2; // MUL A, B (A = A*B)
                break;
            case (Opcode::DIV) :
                result = value2 / value1; // DIV A, B (A = B/A)
                break;
        }

        updateReg(operand1, result);
    }
};

// handles inputOutput command for assembly instructions
class IOInstruction : public Instruction
{
private:
    int operand1;
public:
    IOInstruction(CPU& c, Opcode opc, int opr1) : Instruction(c, opc) 
    { 
        operand1 = opr1;
    }

    void execute() override
    {
        switch (opcode)
        {
            case (Opcode::INPUT) :
            {
                int value;  // read as int then cast to int8_t to avoid reading character
                cout << "?";
                cin >> value;
                updateReg(operand1, static_cast<int8_t>(value));
                break;
            }
            case (Opcode::DISPLAY) :
            {
                int value = static_cast<int>(cpu.getReg(operand1)); // static cast to int to print as number (int8_t will print char)
                cout << "DISPLAY: R[" << operand1 << "]:" << value << endl << endl;
                break;
            }
        } 
    }
};

// handles bitwise operations
class ShiftInstruction : public Instruction
{
private:
    int operand1;
    int count;
public:
    ShiftInstruction(CPU& c, Opcode opc, int opr1, int opr2) : Instruction(c, opc)
    {
        operand1 = opr1;
        count = opr2; 
    } 

    void execute() override
    {
        switch (opcode)
        {
            case (Opcode::ROL) :
            {
                // TODO
            }
            case (Opcode::ROR) :
            {
                // TODO
            }
            case (Opcode::SHL) :
            {
                // tODO
            }
            case (Opcode::SHR) :
            {
                // TODO
            }
        }
    }
};

// handles movement of data between registers and memory
class DataMovementInstruction : public Instruction
{
private:
    int operand1;
    int8_t operand2;
    bool isReg;
    bool indirect;
public:
    DataMovementInstruction(CPU& c, Opcode opc, int opr1, int8_t opr2, bool isR, bool ind=false) : Instruction(c, opc) 
    { 
        operand1 = opr1;
        operand2 = opr2; 
        isReg = isR;
        indirect = ind;
    }

    void execute() override
    {
        switch (opcode)
        {
            case (Opcode::MOV) :
            {
                int8_t value;
                if (indirect) value = cpu.getMem(cpu.getReg(operand2));
                else value = isReg? cpu.getReg(operand2) : operand2;
                updateReg(operand1, value);
                break;
            }
            case (Opcode::LOAD) :
            {
                int memAdr = isReg? cpu.getReg(operand2) : operand2;
                int8_t value = cpu.getMem(memAdr);
                updateReg(operand1, value);
                break;
            }
            case (Opcode::STORE) :
            {
                int memAdr = isReg? cpu.getReg(operand2) : operand2;
                int8_t value = cpu.getReg(operand1);
                cpu.setMem(memAdr, value);
                break;
            }
        }
    }
};

// handles stack operation
class StackInstruction : public Instruction
{
private:
    int operand1;
public:
    StackInstruction(CPU& c, Opcode opc, int opr1) : Instruction(c, opc) { operand1 = opr1; }

    void execute() override
    {
        switch (opcode)
        {
            case (Opcode::PUSH) :
            {
                int SI = cpu.getSI();

                if (SI >= 8)
                {
                    cout << "Error: Pushing to full stack.\n";
                    return;
                }

                int8_t value = cpu.getReg(operand1);
                cpu.pushStack(value);
                break;
            }
            case (Opcode::POP) :
            {
                int SI = cpu.getSI();

                if (SI <= 0)
                {
                    cout << "Error: Popping from empty stack.\n";
                    return;
                }

                int8_t value = cpu.popStack();
                updateReg(operand1, value);
                break;
            }
        }
    }
};

// handles flag reset operation
class RESETInstruction : public Instruction
{
public: 
    Flags flag;

    RESETInstruction(CPU& c, Opcode opc, Flags opr1) : Instruction(c, opc) { flag = opr1; };

    void execute() override
    {
        updateFlag(flag, false);
    }
};

// LIM main for testing 
int main()
{
    // LIM test code for instruction part
    CPU cpu;

    int n = 14;
    Instruction* instructions[n] = {
        new DataMovementInstruction(cpu, Opcode::MOV, 0, 10, false, false),     // 0 R[0] = 10
        new DataMovementInstruction(cpu, Opcode::MOV, 1, 30, false, false),     // 1 R[1] = 30
        new ArithmeticInstruction(cpu, Opcode::ADD, 0, 1, true),                // 2 R[0] = R[0] + R[1] = 40
        new ArithmeticInstruction(cpu, Opcode::DEC, 1),                         // 3 R[1]-- = 29
        new DataMovementInstruction(cpu, Opcode::STORE, 1, 55, false),          // 4 M[55] = R[1] = 29
        new DataMovementInstruction(cpu, Opcode::MOV, 2, 55, false, false),     // 5 R[2] = 55
        new DataMovementInstruction(cpu, Opcode::MOV, 3, 2, true, false),       // 6 R[3] = R[2] = 55
        new DataMovementInstruction(cpu, Opcode::LOAD, 4, 55, false),           // 7 R[4] = M[55] = 29
        new DataMovementInstruction(cpu, Opcode::MOV, 5, 2, true, true),        // 8 R[5] = <R[2]> = M[55] = 29
        new StackInstruction(cpu, Opcode::PUSH, 0),                             // 9 Stack: 10
        new StackInstruction(cpu, Opcode::PUSH, 1),                             // 10 Stack: 40, 29
        new StackInstruction(cpu, Opcode::POP, 6),                              // 11 Stack: 40; R[6] = 29
        new IOInstruction(cpu, Opcode::DISPLAY, 0),                             // 12 R[0] = 10; disp('40')
        new IOInstruction(cpu, Opcode::INPUT, 7)                                // 13 R[1] = (input)
    };

    for (int i = 0; i < n; i++) {
        instructions[i]->execute();
        cpu.dump(i);
        delete instructions[i];
        instructions[i] = nullptr;
    }

    return 0;
}