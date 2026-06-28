#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <cstring>
#include <cstdio> // for sscanf
#include <memory> // for testing
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

enum class ExecutionResult
{
    Success,            // No error
    InvalidRegister,    // Invalid reg (not 0-7)
    InvalidFlag,        // Invalid flag reg (OUCZ)
    MemoryFault,        // Invalid mem address
    DivisionByZero,     // Div 0, A
    PushToFullStack,    // Pushing into full stack(8)
    PopFromEmptyStack,  // Popping from empty stack (0)
    Halt                // End
};

bool handleExecResult(ExecutionResult, int);

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
    void updateFlags(int result, bool isArithmetic = false) 
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
            cf = (result >= 256); // 0-255 (uint8_t)
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
    bool validReg(int n) const { return (n >= 0 && n <= 7); };
    int8_t getReg(int n) const { return reg[n]; }
    void setReg(int n, int8_t val) { reg[n] = val; }
    int8_t getMem(int n) const { return mem[n]; }
    void setMem(int n, int8_t val) { mem[n] = val; }
    void setFlag(int result, bool isArithmetic) 
    {
        // flags->updateFlags(result, isArithmetic);
        testFlags[0] = (result > 127) | testFlags[0];
        testFlags[1] = (result < -128) | testFlags[1];
        testFlags[2] = (result >= 256) | testFlags[2];
        testFlags[3] = (result == 0) | testFlags[3];
    }
    void resetFlag(string flag) 
    {
        // flags->resetFlag(flag)
        if (flag == "of") testFlags[0]=false;
        else if (flag == "uf") testFlags[1]=false;
        else if (flag == "cf") testFlags[2]=false;
        else testFlags[3]=false;
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
    void updateReg(int opr1, int result, bool isArithmetic=false);
    void updateFlag(int result, bool isArithmetic);
public:
    Instruction(CPU& c, Opcode opc);
    virtual ~Instruction() = default; // destructor
    virtual ExecutionResult execute() = 0; // pure virtual: causes classes to implement.
};

// Handles 'ADD', 'SUB' and other arithmetic instructions for assembly
class ArithmeticInstruction : public Instruction
{   
private:
    int operand1;
    int operand2;
    bool isReg;
public:
    ArithmeticInstruction(CPU& c, Opcode opc, int opr1, int opr2=1, bool isR=false);
    ExecutionResult execute() override;
};

// handles inputOutput command for assembly instructions
class IOInstruction : public Instruction
{
private:
    int operand1;
public:
    IOInstruction(CPU& c, Opcode opc, int opr1);
    ExecutionResult execute() override;
};

// handles bitwise operations
class ShiftInstruction : public Instruction
{
private:
    int operand1;
    int count;
    void decimalToBinary(int dec, bool* bits);
    int binaryToDecimal(const bool* bits);
    int getIndex(int idx);  // fucntion to ensure array index between 0-7
public:
    ShiftInstruction(CPU& c, Opcode opc, int opr1, int opr2);
    ExecutionResult execute() override;
};

// handles movement of data between registers and memory
class DataMovementInstruction : public Instruction
{
private:
    int operand1;
    int operand2;    // int operand2 to detect OUZ flag from user input for MOV
    bool isReg;
    bool indirect;
public:
    DataMovementInstruction(CPU& c, Opcode opc, int opr1, int opr2, bool isR, bool ind=false);
    ExecutionResult execute() override;
};

// handles stack operation
class StackInstruction : public Instruction
{
private:
    int operand1;
public:
    StackInstruction(CPU& c, Opcode opc, int opr1);
    ExecutionResult execute() override;
};

// handles flag reset operation
class RESETInstruction : public Instruction
{
private:
    string flag;
public: 
    RESETInstruction(CPU& c, Opcode opc, string opr1);
    ExecutionResult execute() override;
};

// LIM main for testing 
int main()
{
    // LIM test code for instruction part
    CPU cpu;

    std::unique_ptr<Instruction> instructions[] = {
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 0, 10, false, false),     // 0 R[0] = 10
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 1, 30, false, false),     // 1 R[1] = 30
        //std::make_unique<ArithmeticInstruction>(cpu, Opcode::ADD, 0, 1, true),                // 2 R[0] = R[0] + R[1] = 40
        //std::make_unique<ArithmeticInstruction>(cpu, Opcode::DEC, 1),                         // 3 R[1]-- = 29
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::STORE, 1, 55, false),          // 4 M[55] = R[1] = 29
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 2, 55, false, false),     // 5 R[2] = 55
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 3, 2, true, false),       // 6 R[3] = R[2] = 55
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::LOAD, 4, 55, false),           // 7 R[4] = M[55] = 29
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 5, 2, true, true),        // 8 R[5] = <R[2]> = M[55] = 29
        //std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 0),                             // 9 Stack: 10
        //std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 1),                             // 10 Stack: 40, 29
        //std::make_unique<StackInstruction>(cpu, Opcode::POP, 6),                              // 11 Stack: 40; R[6] = 29
        //std::make_unique<IOInstruction>(cpu, Opcode::DISPLAY, 0),                             // 12 R[0] = 10; disp('40')
        //std::make_unique<IOInstruction>(cpu, Opcode::INPUT, 7),                               // 13 R[1] = (input)
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 1, 88, false, false),
        //std::make_unique<ShiftInstruction>(cpu, Opcode::SHL, 1, 3),
        //std::make_unique<ShiftInstruction>(cpu, Opcode::SHR, 1, 2),
        //std::make_unique<ShiftInstruction>(cpu, Opcode::ROL, 1, 7),
        //std::make_unique<ShiftInstruction>(cpu, Opcode::ROR, 1, 10),
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 0, 1, false, false),
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 0, 0, false, false),
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 0, -129, false, false),
        //std::make_unique<RESETInstruction>(cpu, Opcode::RESET, "u"),
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 0, 128, false, false),
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 1, 127, false, false),
        //std::make_unique<ArithmeticInstruction>(cpu, Opcode::ADD, 1, 200, false)
        //std::make_unique<DataMovementInstruction>(cpu, Opcode::MOV, 1, 0, false, false),
        //std::make_unique<ArithmeticInstruction>(cpu, Opcode::DIV, 1, 100, false),
        //std::make_unique<IOInstruction>(cpu, Opcode::INPUT, 8),
        //std::make_unique<RESETInstruction>(cpu, Opcode::RESET, "someflag"),
        //std::make_unique<StackInstruction>(cpu, Opcode::POP, 1)
        std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 1),
        std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 1),
        std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 1),
        std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 1),
        std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 1),
        std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 1),
        std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 1),
        std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 1),
        std::make_unique<StackInstruction>(cpu, Opcode::PUSH, 1)
    };

    int i = 1;
    for (auto& inst : instructions) 
    {
        ExecutionResult execResult = inst->execute();
        bool running = handleExecResult(execResult, i);
        if (running) cpu.dump(i);
        else cout << "\n\"program crashed\"\n";
        i++;
    }

    return 0;
}

// helper function to handle execution result and errors
bool handleExecResult(ExecutionResult execResult, int pc)
{
    switch (execResult)
    {
        case (ExecutionResult::Success) :
            return true;
        case (ExecutionResult::Halt) :
            cout << "Execution completed.";
            return false;
        case (ExecutionResult::InvalidRegister) :
            cerr << "Error: Invalid Register in Instruction " << pc;
            break;
        case (ExecutionResult::InvalidFlag) :
            cerr << "Error: Invalid Flag Register in Instruction " << pc;
            break;  
        case (ExecutionResult::MemoryFault) :
            cerr << "Error: Invalid Memory Address in Instruction " << pc;
            break;  
        case (ExecutionResult::DivisionByZero) :
            cerr << "Error: Division by Zero in Instruction " << pc;
            break;  
        case (ExecutionResult::PushToFullStack) :
            cerr << "Error: Pushing to full Stack in Instruction " << pc;
            break;  
        case (ExecutionResult::PopFromEmptyStack) :
            cerr << "Error: Popping from empty Stack in Instruction " << pc;
            break;      
    }
    return false;
}

// function defintion for instruction base and derived class
void Instruction::updateReg(int opr1, int result, bool isArithmetic)
{
    updateFlag(result, isArithmetic);
    cpu.setReg(opr1, static_cast<int8_t>(result));
}

void Instruction::updateFlag(int result, bool isArithmetic)
{ 
    cpu.setFlag(result, isArithmetic); 
}

Instruction::Instruction(CPU& c, Opcode opc) 
    : cpu(c), 
    opcode(opc) 
{}

ArithmeticInstruction::ArithmeticInstruction(CPU& c, Opcode opc, int opr1, int opr2, bool isR) 
    : Instruction(c, opc), 
    operand1(opr1), 
    operand2(opr2), // int data type operand2 to detect OUZ flag from user input 
    isReg(isR)
{}

ExecutionResult ArithmeticInstruction::execute()
{
    if (!cpu.validReg(operand1)) return ExecutionResult::InvalidRegister;   // return error if invalid reg

    int result;
    int8_t value1 = cpu.getReg(operand1);
    int value2 = isReg? cpu.getReg(operand2) : operand2; // get value from register[opr2] if is reg  

    switch (opcode)
    {
        case (Opcode::ADD) :
        case (Opcode::INC) :
            result = value1 + value2;   // ADD A, B (A = A+B); INC A (A = A+1)
            break;
        case (Opcode::SUB) :
        case (Opcode::DEC) :
            result = value1 - value2;   // SUB A, B (A = A-B); DEC A (A = A-1)
            break;
        case (Opcode::MUL) :
            result = value1 * value2;   // MUL A, B (A = A*B)
            break;
        case (Opcode::DIV) :
            if (!value1) return ExecutionResult::DivisionByZero;    // raise error if value1 is 0 
            result = value2 / value1;   // DIV A, B (A = B/A)
            break;
    }

    updateReg(operand1, result, true);
    return ExecutionResult::Success;
}

IOInstruction::IOInstruction(CPU& c, Opcode opc, int opr1) 
    : Instruction(c, opc),
    operand1(opr1)
{}

ExecutionResult IOInstruction::execute()
{
    if (!cpu.validReg(operand1)) return ExecutionResult::InvalidRegister;   // return error if invalid reg

    switch (opcode)
    {
        case (Opcode::INPUT) :
        {
            int value;  // read as int then cast to int8_t to avoid reading character
            cout << "?";
            cin >> value;
            updateReg(operand1, value);
            break;
        }
        case (Opcode::DISPLAY) :
        {
            int value = static_cast<int>(cpu.getReg(operand1)); // static cast to int to print as number (int8_t will print char)
            cout << "DISPLAY: R[" << operand1 << "]:" << value << endl << endl;
            break;
        }
    } 

    return ExecutionResult::Success;
}

void ShiftInstruction::decimalToBinary(int dec, bool* bits)
{
    for (int i=0; i<8; i++)
    {
        bits[i] = dec % 2;    // bit i = decimal/(2^i) %2
        dec /= 2;
    }        
}

int ShiftInstruction::binaryToDecimal(const bool* bits)
{
int dec  = 0;
int bitValue = 1;

    for (int i=0; i<8; i++)
    {
        if (bits[i]) dec += bitValue;   // if binary bit = 1; add 2^i to decimal
        bitValue *= 2;
    }
}

int ShiftInstruction::getIndex(int idx)
{
    if (idx > 7) return (idx-8);
    else if (idx < 0) return (idx+8);
    else return idx;
}
    
ShiftInstruction::ShiftInstruction(CPU& c, Opcode opc, int opr1, int opr2) 
    : Instruction(c, opc),
    operand1(opr1),
    count(opr2 % 8) // range of shifting is 0-7 bits
{} 

ExecutionResult ShiftInstruction::execute()
{
    if (!cpu.validReg(operand1)) return ExecutionResult::InvalidRegister;   // return error if invalid reg

    uint8_t dec = static_cast<uint8_t>(cpu.getReg(operand1));   // cast to unsigned byte to avoid negative value
    bool binaryBits[8] = {};
    bool resultBits[8] = {};
    decimalToBinary(dec, binaryBits);

    switch (opcode)
    {
        case (Opcode::ROL) :
            for (int i=0; i<8; i++) resultBits[i] = binaryBits[getIndex(i-count)];  // result bit = binary bit from right (wrap around)
            break;
        case (Opcode::ROR) :
            for (int i=0; i<8; i++) resultBits[i] = binaryBits[getIndex(i+count)];  // result bit = binary bit from left (wrap around)
            break;
        case (Opcode::SHL) :
            for (int i=count; i<8; i++) resultBits[i] = binaryBits[getIndex(i-count)];  // result bit = binary bit from right, remaining 0
            break;
        case (Opcode::SHR) :
            for (int i=0; i<(8-count); i++) resultBits[i] = binaryBits[getIndex(i+count)];  // result bit = binary bit from left, remaining 0
            break;
    }

    int value = binaryToDecimal(resultBits);
    updateReg(operand1, value);
    return ExecutionResult::Success;
}

DataMovementInstruction::DataMovementInstruction(CPU& c, Opcode opc, int opr1, int opr2, bool isR, bool ind) 
    : Instruction(c, opc), 
    operand1(opr1), 
    operand2(opr2), // int data type operand2 to detect OUZ flag from user input 
    isReg(isR),
    indirect(ind)
{}

ExecutionResult DataMovementInstruction::execute()
{
    switch (opcode)
    {
        case (Opcode::MOV) :
        {
            int value;
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

    return ExecutionResult::Success;
}

StackInstruction::StackInstruction(CPU& c, Opcode opc, int opr1) 
    : Instruction(c, opc),
    operand1(opr1)
{}

ExecutionResult StackInstruction::execute()
{
    switch (opcode)
    {
        case (Opcode::PUSH) :
        {
            int SI = cpu.getSI();

            if (SI >= 8) return ExecutionResult::PushToFullStack;

            int8_t value = cpu.getReg(operand1);
            cpu.pushStack(value);
            break;
        }
        case (Opcode::POP) :
        {
            int SI = cpu.getSI();

            if (SI <= 0) return ExecutionResult::PopFromEmptyStack;

            int8_t value = cpu.popStack();
            updateReg(operand1, value);
            break;
        }
    }
    return ExecutionResult::Success;
}

RESETInstruction::RESETInstruction(CPU& c, Opcode opc, string opr1) 
    : Instruction(c, opc),
    flag(opr1)
{}

ExecutionResult RESETInstruction::execute()
{
    if (flag=="of" || flag=="uf" || flag=="cf" || flag=="zf")
    {
        cpu.resetFlag(flag);
        return ExecutionResult::Success;
    }
    else return ExecutionResult::InvalidFlag;
}