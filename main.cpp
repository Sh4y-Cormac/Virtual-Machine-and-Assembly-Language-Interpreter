#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <cstring>
#include <iomanip> // for setw and fill
#include <cstdio> // for sscanf
using namespace std;

enum class Flags
{
    OF, UF, CF, ZF, COUNT
};

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
    InvalidInstruction, // Invalid opcode
    InvalidRegister,    // Invalid reg (not 0-7)
    InvalidFlag,        // Invalid flag reg (OUCZ)
    MemoryFault,        // Invalid mem address
    DivisionByZero,     // Div 0, A
    PushToFullStack,    // Pushing into full stack(8)
    PopFromEmptyStack   // Popping from empty stack (0)
};

bool handleExecResult(ExecutionResult, int);

// MADE BY UMAR
// Base class of register (Encapsulates an 8-bit signed value and flag update logic) 
class Register
{
protected:
    int8_t value; // value of the 8-bit itself

public:
    Register() : value()
    {}

    void setValue(int8_t val)
    {
        value = val;
    }

    int8_t getValue() const
    {
        return value;
    }
};

// MADE BY UMAR
// implementation of the register movement  (inheritance from Register)
class GeneralRegister : public Register
{
    public:
        GeneralRegister()
        {
            // base class constructor already initializes to 0,
            // just for best practice to be explicit in derived classes
            value = 0;
        }

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

    // Getter for display formatting
    bool getFlag(Flags flag) const
    {
        if (flag == Flags::OF) return of;
        else if (flag == Flags::UF) return uf;
        else if (flag == Flags::CF) return cf;
        else return zf;
    }

    // Manual manual reset instruction logic
    void resetFlag(Flags flagName) 
    {
        if (flagName == Flags::CF) cf = false;
        else if (flagName == Flags::ZF) zf = false;
        else if (flagName == Flags::UF) uf = false;
        else if (flagName == Flags::OF) of = false;
    }

    // Core validation method to check arithmetic bounds
    void updateFlags(int result, bool isArithmetic) 
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

// MADE BY UMAR
// Handles storage and addressing logic over a vector of bytes
class Memory
{
private:
    static const size_t Memory_Size = 64; // define the fixed size of the memory
    int8_t storage[Memory_Size]; // making the array with the fixed size

public:
    Memory()
    {
        // initialize all index to 0 (constructor)
        for (size_t i = 0; i < Memory_Size; i++)
        {
            storage[i] = 0;
        }
    }
    void write(size_t address, int8_t value)
    {
        if (address < Memory_Size)
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
        if (address < Memory_Size)
        {
            return storage[address];
        }
        cerr << "Memory Access Violation: Read at " << address << endl;
        return 0;
    }
};

// Contains registers, memory, PC, executes instructions 
class CPU
{
private:
    Memory memory;          
    FlagRegister* flags;    
    GeneralRegister* registers;
    int pc=0;

    // Your Assigned Additions to align with requirements:
    int8_t stackStorage[8]; // The 8-byte system stack managed internally
    int si=0;               // Stack Index (SI) register starting at 0

public:
    // cpu functions done by LIM
    CPU(FlagRegister* flag, GeneralRegister* reg) : flags(flag), registers(reg) {}

    int8_t getMemory(int memAdr) const { return memory.read(memAdr); }
    bool getFlag(Flags flag) const { return flags->getFlag(flag); }
    int8_t getRegister(int regNum) const { return registers[regNum].getValue(); }
    int getPC() const { return pc; }

    void setRegister(int regNum, int8_t value) { registers[regNum].setValue(value); }
    void updateFlags(int result, bool isArithmetic) { flags->updateFlags(result, isArithmetic); }
    void resetFlag(Flags flag) { flags->resetFlag(flag); }
    void setMemory(int memAdr, int8_t value) { memory.write(memAdr, value); }

    void pushStack(int8_t value) { stackStorage[si++] = value; }
    int8_t popStack() { return stackStorage[--si]; }
    int getSI() const { return si; }

    bool isValidReg(int n) const { return (n >= 0 && n <= 7); };
    bool isValidMem(int n) const { return (n >= 0 && n <= 63); };

    void dump() const;
};

// Loads programs, decodes instructions, delegates execution to `CPU`
class ParsedCommand
{
private:
    string opcode;
    string operand1;
    string operand2;
public:   
    ParsedCommand(string opc, string opr1="", string opr2="") 
        : opcode(opc), operand1(opr1), operand2(opr2) {}
    string getOpcode() const { return opcode; }
    string getOperand1() const { return operand1; }
    string getOperand2() const { return operand2; }
};

class Runner
{
private:
    CPU* cpu;
    ParsedCommand* programs;
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

    void parseLine(char line[], ParsedCommand& inst) {
        
    }

    bool loadProgram(const char fileName[]) {
        return true;
    }

    void displayInstruction(ParsedCommand inst) {
        
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
    void updateFlags(int result, bool isArithmetic);
public:
    Instruction(CPU& c, Opcode opc);
    virtual ~Instruction() = default; // destructor
    virtual ExecutionResult execute() = 0; // pure virtual: causes classes to implement.
};

// Handles 'ADD', 'SUB' and other arithmetic instructions for assembly (polymorphism of Instuction)
class ArithmeticInstruction : public Instruction
{   
private:
    int operand1;
    int operand2;
    bool isReg;
    ExecutionResult add(int& result, int val1, int val2);
    ExecutionResult sub(int& result, int val1, int val2);
    ExecutionResult mul(int& result, int val1, int val2);
    ExecutionResult div(int& result, int val1, int val2);
public:
    ArithmeticInstruction(CPU& c, Opcode opc, int opr1, int opr2=1, bool isR=false);
    ExecutionResult execute() override;
};

// handles inputOutput command for assembly instructions (polymorphism of Instuction)
class IOInstruction : public Instruction
{
private:
    int operand1;
    void input();
    void display() const;
public:
    IOInstruction(CPU& c, Opcode opc, int opr1);
    ExecutionResult execute() override;
};

// handles bitwise operations (polymorphism of Instuction)
class ShiftInstruction : public Instruction
{
private:
    int operand1;
    int count;
    void decimalToBinary(int dec, bool* bits);
    int binaryToDecimal(const bool* bits);
    int getIndex(int idx);  // fucntion to ensure array index between 0-7
    void ROL(bool* const binaryBits, bool* resultBits);
    void ROR(bool* const binaryBits, bool* resultBits);
    void SHL(bool* const binaryBits, bool* resultBits);
    void SHR(bool* const binaryBits, bool* resultBits);
public:
    ShiftInstruction(CPU& c, Opcode opc, int opr1, int opr2);
    ExecutionResult execute() override;
};

// handles movement of data between registers and memory (polymorphism of Instuction)
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
    ExecutionResult mov();
    ExecutionResult load();
    ExecutionResult store();
};

// handles stack operation (polymorphism of Instuction)
class StackInstruction : public Instruction
{
private:
    int operand1;
    ExecutionResult push();
    ExecutionResult pop();
public:
    StackInstruction(CPU& c, Opcode opc, int opr1);
    ExecutionResult execute() override;
};

// handles flag reset operation (polymorphism of Instuction)
class RESETInstruction : public Instruction
{
private:
    Flags flag;
public: 
    RESETInstruction(CPU& c, Opcode opc, Flags opr1);
    ExecutionResult execute() override;
};

int main()
{
    // initialize the 64 bit memory
    Memory myMemory;

    //initialize the flag register
    FlagRegister myFlags;

    // initialize the R0-R7 general registers array
    GeneralRegister myRegisters[8];

    // Send everything to the CPU for processing
    CPU myCPU(&myFlags, myRegisters);

    // testing script
    myCPU.dump();
    
    return 0;
}

// helper function to handle execution result and errors
bool handleExecResult(ExecutionResult execResult, int pc)
{
    switch (execResult)
    {
        case (ExecutionResult::Success) :
            return true;
        case (ExecutionResult::InvalidInstruction) :
            cerr << "Error: Invalid Instruction Opcode in Instruction " << pc;
            break;
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

// function defintion for cpu.dump
void CPU::dump() const
{
    string flagName[] = {"OF", "UF", "CF", "ZF"};

    cout << "#Begin#\n";

    cout << "#Registers#";
    for (int i = 0; i < 8; i++) 
        cout << setw(4) << setfill('0') << static_cast<int>(getRegister(i)) << "#";

    cout << "\n#Flags#";
    for (int i = 0; i < static_cast<int>(Flags::COUNT); i++) 
        cout << flagName[i] << "#" << static_cast<int>(getFlag(static_cast<Flags>(i))) << "#";

    cout << "\n#PC#" << setw(4) << setfill('0') << getPC() << "#";

    cout << "\n#Memory#\n#";
    for (int i = 0; i < 64; i++) 
    {
        cout << setw(4) << setfill('0') << static_cast<int>(getMemory(i)) << "#";
        if (!((i+1)%8)) cout << "\n#";
    }

    cout << "End#\n";
}

// function defintion for instruction base and derived class
void Instruction::updateReg(int opr1, int result, bool isArithmetic)
{
    updateFlags(result, isArithmetic);
    cpu.setRegister(opr1, static_cast<int8_t>(result));
}

void Instruction::updateFlags(int result, bool isArithmetic)
{ 
    cpu.updateFlags(result, isArithmetic); 
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
    if (!cpu.isValidReg(operand1)) return ExecutionResult::InvalidRegister;   // return error if invalid reg
    if (isReg && !cpu.isValidReg(operand2)) return ExecutionResult::InvalidRegister;   // return error if invalid reg

    int result;
    ExecutionResult execResult;
    int8_t value1 = cpu.getRegister(operand1);
    int value2 = isReg? cpu.getRegister(operand2) : operand2; // get value from register[opr2] if is reg  

    switch (opcode)
    {
        case (Opcode::ADD) : 
        case (Opcode::INC) :
            execResult = add(result, value1, value2);
            break;
        case (Opcode::SUB) : 
        case (Opcode::DEC) :
            execResult = sub(result, value1, value2);
            break;
        case (Opcode::MUL) :
            execResult = mul(result, value1, value2);
            break;
        case (Opcode::DIV) :
            execResult = div(result, value1, value2);
            break;
        default:
            execResult = ExecutionResult::InvalidInstruction;
            break;
    }

    if (execResult == ExecutionResult::Success) updateReg(operand1, result, true);
    return execResult;
}

ExecutionResult ArithmeticInstruction::add(int& result, int val1, int val2)
{ 
    result = val1 + val2;   // ADD A, B (A = A+B); INC A (A = A+1) 
    return ExecutionResult::Success;
}

ExecutionResult ArithmeticInstruction::sub(int& result, int val1, int val2)
{ 
    result = val1 - val2;   // SUB A, B (A = A-B); DEC A (A = A-1)
    return ExecutionResult::Success;
}

ExecutionResult ArithmeticInstruction::mul(int& result, int val1, int val2)
{ 
    result = val1 * val2;   // MUL A, B (A = A*B)
    return ExecutionResult::Success;
}   

ExecutionResult ArithmeticInstruction::div(int& result, int val1, int val2)
{ 
    if (!val1) return ExecutionResult::DivisionByZero;    // raise error if value1 is 0 
    result = val2 / val1;   // DIV A, B (A = B/A)
    return ExecutionResult::Success;
}

IOInstruction::IOInstruction(CPU& c, Opcode opc, int opr1) 
    : Instruction(c, opc),
    operand1(opr1)
{}

ExecutionResult IOInstruction::execute()
{
    if (!cpu.isValidReg(operand1)) return ExecutionResult::InvalidRegister;   // return error if invalid reg

    switch (opcode)
    {
        case (Opcode::INPUT) :
            input();
            break;
        case (Opcode::DISPLAY) :
            display();
            break;
        default:
            return ExecutionResult::InvalidInstruction;
    } 

    return ExecutionResult::Success;
}

void IOInstruction::input()
{
    int value;  // read as int then cast to int8_t to avoid reading character
    cout << "?";
    cin >> value;
    updateReg(operand1, value);
}

void IOInstruction::display() const
{
    int value = static_cast<int>(cpu.getRegister(operand1)); // static cast to int to print as number (int8_t will print char)
    cout << "DISPLAY: R[" << operand1 << "]:" << value << endl << endl;
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

    return dec;
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
    if (!cpu.isValidReg(operand1)) return ExecutionResult::InvalidRegister;   // return error if invalid reg

    uint8_t dec = static_cast<uint8_t>(cpu.getRegister(operand1));   // cast to unsigned byte to avoid negative value
    bool binaryBits[8] = {};
    bool resultBits[8] = {};
    decimalToBinary(dec, binaryBits);

    switch (opcode)
    {
        case (Opcode::ROL) :
            ROL(binaryBits, resultBits);
            break;
        case (Opcode::ROR) :
            ROR(binaryBits, resultBits);
            break;
        case (Opcode::SHL) :
            SHL(binaryBits, resultBits);
            break;
        case (Opcode::SHR) :
            SHR(binaryBits, resultBits);
            break;
        default:
            return ExecutionResult::InvalidInstruction;
    }

    int value = binaryToDecimal(resultBits);
    updateReg(operand1, value);
    return ExecutionResult::Success;
}

void ShiftInstruction::ROL(bool* const binaryBits, bool* resultBits)
{
    for (int i=0; i<8; i++) 
    {
        resultBits[i] = binaryBits[getIndex(i-count)];  // result bit = binary bit from right (wrap around)
    }
}

void ShiftInstruction::ROR(bool* const binaryBits, bool* resultBits)
{
    for (int i=0; i<8; i++) 
    {
        resultBits[i] = binaryBits[getIndex(i+count)];  // result bit = binary bit from left (wrap around)
    }
}

void ShiftInstruction::SHL(bool* const binaryBits, bool* resultBits)
{
    for (int i=count; i<8; i++) 
    {
        resultBits[i] = binaryBits[getIndex(i-count)];  // result bit = binary bit from right, remaining 0
    }
}

void ShiftInstruction::SHR(bool* const binaryBits, bool* resultBits)
{
    for (int i=0; i<(8-count); i++) 
    {
        resultBits[i] = binaryBits[getIndex(i+count)];  // result bit = binary bit from left, remaining 0
    }
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
    if (!cpu.isValidReg(operand1)) return ExecutionResult::InvalidRegister;   // return error if invalid reg
    if (isReg && !cpu.isValidReg(operand2)) return ExecutionResult::InvalidRegister;   // return error if invalid reg

    switch (opcode)
    {
        case (Opcode::MOV) :
            return mov();
        case (Opcode::LOAD) :
            return load();
        case (Opcode::STORE) :
            return store();
        default:
            return ExecutionResult::InvalidInstruction;
    }
}

ExecutionResult DataMovementInstruction::mov()
{
    int value;

    if (indirect) 
    {
        int memAdr = cpu.getRegister(operand2);
        if (!cpu.isValidMem(memAdr)) return ExecutionResult::MemoryFault;   // return error if invalid mem adr
        value = cpu.getMemory(memAdr);   
    }
    else value = isReg? cpu.getRegister(operand2) : operand2;

    updateReg(operand1, value);
    return ExecutionResult::Success;
}

ExecutionResult DataMovementInstruction::load()
{
    int memAdr = isReg? cpu.getRegister(operand2) : operand2;
    if (!cpu.isValidMem(memAdr)) return ExecutionResult::MemoryFault;   // return error if invalid mem adr

    int8_t value = cpu.getMemory(memAdr);
    updateReg(operand1, value);
    return ExecutionResult::Success;
}

ExecutionResult DataMovementInstruction::store()
{
    int memAdr = isReg? cpu.getRegister(operand2) : operand2;
    if (!cpu.isValidMem(memAdr)) return ExecutionResult::MemoryFault;   // return error if invalid mem adr

    int8_t value = cpu.getRegister(operand1);
    cpu.setMemory(memAdr, value);
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
            return push();
        case (Opcode::POP) :
            return push(); 
        default:
            return ExecutionResult::InvalidInstruction;
    }
}

ExecutionResult StackInstruction::push()
{
    int SI = cpu.getSI();

    if (SI >= 8) return ExecutionResult::PushToFullStack;

    int8_t value = cpu.getRegister(operand1);
    cpu.pushStack(value);
    return ExecutionResult::Success;
}

ExecutionResult StackInstruction::pop()
{
    int SI = cpu.getSI();

    if (SI <= 0) return ExecutionResult::PopFromEmptyStack;

    int8_t value = cpu.popStack();
    updateReg(operand1, value);
    return ExecutionResult::Success;
}

RESETInstruction::RESETInstruction(CPU& c, Opcode opc, Flags opr1) 
    : Instruction(c, opc),
    flag(opr1)
{}

ExecutionResult RESETInstruction::execute()
{
    if (flag==Flags::OF || flag==Flags::UF || flag==Flags::CF || flag==Flags::ZF)
    {
        cpu.resetFlag(flag);
        return ExecutionResult::Success;
    }
    else return ExecutionResult::InvalidFlag;
}