#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <cstring>
#include <sstream>
#include <iomanip> // for setw and fill to format output
#include <cstdio> // for sscanf
#include <sstream>
using namespace std;

// MADE BY ZHEN LONG
// Type-safe constants for identifying flags in flag register
enum class Flags
{
    OF, UF, CF, ZF, COUNT
};

// MADE BY ZHEN LONG
// Type-safe constants for identifying opcode in instructions
enum class Opcode
{ 
    ADD, SUB, MUL, DIV, INC, DEC, // Arithmetic
    INPUT, DISPLAY, // IO
    MOV, LOAD, STORE, // DataMovement
    RESET, // Status
    PUSH, POP, // Stack
    SHL, SHR, ROL, ROR, // Shift
    COUNT
};

// MADE BY ZHEN LONG
// Type-safe constants for identifying results and errors after executig instructions
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

// MADE BY ZHEN LONG
// custom stack
class MyStack
{
private:
    int8_t* data;   // The byte system stack managed internally
    int si;         // Stack Index (SI) register starting at 0
    int capacity;   // capacity is 8 byte

public:
    // constructor to initialise 8byte of stack storage
    MyStack(int cap=8) 
    : capacity(cap), si(0) 
    { data = new int8_t[cap](); }  // initialise stack of size 8, all data to zero

    // custom destructor to delete dynamically allocated memory space
    ~MyStack() { delete[] data; }

    // methods to perform operation on internal stack (Encapsulation) 
    void push(int8_t value) { data[si++] = value; }
    int8_t pop() { return data[--si]; }
    int getSI() const { return si; }
};

// MADE BY ZHEN LONG
// custom vector template
template<class T>
class MyVector
{
private:
    T** data; // pointer to an array of pointer of type T
    int capacity;
    int size;

    // increase capacity (dynamic array/vector)
    void resize() 
    {
        capacity *= 2;
        T** newData = new T*[capacity];
        for (int i = 0; i < size; i++) 
        { newData[i] = data[i]; }
        delete[] data;
        data = newData;
    }

public:
    // constructor to initialise vector 
    MyVector(int cap=10) 
    : capacity(cap), size(0)
    { data = new T*[capacity](); } // nullptr

    // destructor to delete each data and the class memory space
    ~MyVector() 
    {
        for (int i = 0; i < size; i++) { delete data[i]; }
        delete[] data;
    }

    // add new instruction
    void pushBack(T* inst) 
    {
        if (size == capacity) 
        { resize(); }
        data[size++] = inst;
    }

    // overload [] operator to return data at index
    T* operator[](int index) { return data[index]; }

    // return size of vector (instruction count)
    int getSize() { return size; }
};

// MADE BY ZHEN LONG
// helper function to handle execution result and displaying errors
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
// Done by Kar Fung
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
        zf = (result == 0);
        of = (result > 127);
        uf = (result < -128);
        if (isArithmetic) 
        {
            cf = (result >= 256); 
            cf = (result >= 256);
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
    ExecutionResult write(size_t address, int8_t value)
    {
        if (address < Memory_Size)
        {
            storage[address] = value;
            return ExecutionResult::MemoryFault;
        }
        else
        {
            cerr << "Memory Access Violation: Write at " << address << endl;
            return ExecutionResult::MemoryFault;
        }
    }
    ExecutionResult read(size_t address, int &value) const
    {
        if (address < Memory_Size)
        {
            value = storage[address];
            return ExecutionResult::MemoryFault;
        }
        cerr << "Memory Access Violation: Read at " << address << endl;
        return ExecutionResult::MemoryFault;
    }
};

// Contains registers, memory, PC, executes instructions 
class CPU
{
private:
    Memory memory;          
    FlagRegister* flags;    
    GeneralRegister* registers;
    MyStack stack;  // cpu internal stack (implementation of stack)
    int pc=0;

public:
    // CPU functions done by ZHEN LONG
    // constuctor to initialise flag and registers inside CPU
    // CPU owns (composition) Memory, but (aggregation) on other component
    CPU(FlagRegister* flag, GeneralRegister* reg) : flags(flag), registers(reg) {}

    // getters to read PRIVATE members data (Encapsulation) 
    ExecutionResult getMemory(int memAdr, int &value) const { return memory.read(memAdr, value); }
    bool getFlag(Flags flag) const { return flags->getFlag(flag); }
    int8_t getRegister(int regNum) const { return registers[regNum].getValue(); }
    int getPC() const { return pc; }
    int getSI() const { return stack.getSI(); }

    // setters to change PRIVATE members data(Encapsulation) 
    void setRegister(int regNum, int8_t value) { registers[regNum].setValue(value); }
    void updateFlags(int result, bool isArithmetic) { flags->updateFlags(result, isArithmetic); }
    void resetFlag(Flags flag) { flags->resetFlag(flag); }
    ExecutionResult setMemory(int memAdr, int8_t value) { return memory.write(memAdr, value); }
    void incrementPC() { pc++; }

    // methods to perform operation on internal stack (Encapsulation) 
    void pushStack(int8_t value) { stack.push(value); }
    int8_t popStack() { return stack.pop(); }

    // validate register number (0 to 7) 
    bool isValidReg(int n) const { return (n >= 0 && n <= 7); };

    // display content of CPU after execution
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
    ParsedCommand() : opcode(""), operand1(""), operand2("") {}
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
    MyVector<ParsedCommand> programs;
    int instructionCount;
    MyVector<Instruction> instructions;

    // Methods to format user input into information
    bool isRegister(string text);
    int getRegisterNumber(string text);
    bool isBracket(string text);
    string removeBracket(string text);
    int getNumber(string text);
    Flags getFlagName(string text);
    Opcode getOpcodeName(string text);

    // Methods to load instructions into vector
    Instruction* loadArithmeticInst(Opcode opc, string opr1, string opr2);
    Instruction* loadIOInst(Opcode opc, string opr1);
    Instruction* loadShiftInst(Opcode opc, string opr1, string opr2);
    Instruction* loadDataMovementInst(Opcode opc, string opr1, string opr2);
    Instruction* loadMOVInst(string opr1, string opr2);
    Instruction* loadLOADInst(string opr1, string opr2);
    Instruction* loadSTOREInst(string opr1, string opr2);
    Instruction* loadStackInst(Opcode opc, string opr1);
    Instruction* loadResetInst(Opcode opc, string opr1);

public:
    Runner(CPU* c); // constructor to initialize cpu

    // methods to read line by line from .asm file and extract word by words
    void cleanLine(char line[]);
    void parseLine(char line[], ParsedCommand& inst);
    bool loadProgram(const char fileName[]);

    // decode program into executable instruction
    ExecutionResult decodeProgram();

    // load instruction into specific dervied instructions class
    void loadInstruction(Opcode opc, string opr1, string opr2);

    void run();
};

// MADE BY ZHEN LONG
// all assembly commands logic go here e.g 'ADD' or 'MOV' (abstract base class)
class Instruction
{
protected:
    CPU& cpu;
    Opcode opcode;
    void updateReg(int opr1, int result, bool isArithmetic=false);  // call setter to update register value after execute
    void updateFlags(int result, bool isArithmetic);    // call setter to update flags value after execute if content destination register is changed
public:
    Instruction(CPU& c, Opcode opc); // constructor to initialize cpu by reference and opcode
    virtual ~Instruction() = default; // calls derived classes destuctor, then uses default base class destructor
    virtual ExecutionResult execute() = 0; // pure virtual function: forces derived classes to implement. (polymorphism)
};

// MADE BY ZHEN LONG
// Handles 'ADD', 'SUB' and other arithmetic instructions for assembly (Inheritance and polymorphism of Instuction)
class ArithmeticInstruction : public Instruction
{   
private:
    int operand1;
    int operand2;
    bool isReg;
    
    // methods to perform arithmetic operation on two values
    ExecutionResult add(int& result, int val1, int val2);
    ExecutionResult sub(int& result, int val1, int val2);
    ExecutionResult mul(int& result, int val1, int val2);
    ExecutionResult div(int& result, int val1, int val2);

public:
    ArithmeticInstruction(CPU& c, Opcode opc, int opr1, int opr2=1, bool isR=false); // constuctor to initialise data and call base class constructor
    ExecutionResult execute() override; // ensure that execute override function in base class (polymorphism)
};

// MADE BY ZHEN LONG
// handles inputOutput command for assembly instructions (Inheritance and polymorphism of Instuction)
class IOInstruction : public Instruction
{
private:
    int operand1;

    // methods to perform input and output operation
    void input();
    void display() const;

public:
    IOInstruction(CPU& c, Opcode opc, int opr1); // constuctor to initialise data and call base class constructor
    ExecutionResult execute() override; // ensure that execute override function in base class (polymorphism)
};

// MADE BY ZHEN LONG
// handles bitwise operations (Inheritance and polymorphism of Instuction)
class ShiftInstruction : public Instruction
{
private:
    int operand1;
    int count;

    // methods to perform binary and decimal conversion
    void decimalToBinary(int dec, bool* bits);
    int binaryToDecimal(const bool* bits);

    int getIndex(int idx);  // methods to ensure array index between 0-7 (8-bits)

    // methods to perform shift and rotate operation on 8 bits binary
    void ROL(bool* const binaryBits, bool* resultBits);
    void ROR(bool* const binaryBits, bool* resultBits);
    void SHL(bool* const binaryBits, bool* resultBits);
    void SHR(bool* const binaryBits, bool* resultBits);

public:
    ShiftInstruction(CPU& c, Opcode opc, int opr1, int opr2); // constuctor to initialise data and call base class constructor
    ExecutionResult execute() override; // ensure that execute override function in base class (polymorphism)
};

// MADE BY ZHEN LONG
// handles movement of data between registers and memory (Inheritance and polymorphism of Instuction)
class DataMovementInstruction : public Instruction
{
private:
    int operand1;
    int operand2;    // int datatype to detect OUZ flag from user input for MOV
    bool isReg;
    bool indirect;

    // methods to perform data movement operation between registers to registers or to memory
    ExecutionResult mov();
    ExecutionResult load();
    ExecutionResult store();

public:
    // constuctor to initialise data and call base class constructor
    DataMovementInstruction(CPU& c, Opcode opc, int opr1, int opr2, bool isR, bool ind=false);

    ExecutionResult execute() override; // ensure that execute override function in base class (polymorphism)
};

// MADE BY ZHEN LONG
// handles stack operation (Inheritance and polymorphism of Instuction)
class StackInstruction : public Instruction
{
private:
    int operand1;

    // methods to perform push and pop operation on CPU internal stack
    ExecutionResult push();
    ExecutionResult pop();

public:
    StackInstruction(CPU& c, Opcode opc, int opr1); // constuctor to initialise data and call base class constructor
    ExecutionResult execute() override; // ensure that execute override function in base class (polymorphism)
};

// MADE BY ZHEN LONG
// handles flag reset operation (Inheritance and polymorphism of Instuction)
class RESETInstruction : public Instruction
{
private:
    Flags flag;
public: 
    RESETInstruction(CPU& c, Opcode opc, Flags opr1); // constuctor to initialise data and call base class construct
    ExecutionResult execute() override; // ensure that execute override function in base class (polymorphism)
};

int main()
{
    // *****************************************************************
    // NOTE FOR ALSAKKAF : FEEL FREE TO CHANGE THE MAIN TO FIT YOUR RUNNER
    // *****************************************************************
    // initialize the 64 bit memory
    Memory myMemory;

    //initialize the flag register
    FlagRegister myFlags;

    // initialize the R0-R7 general registers array
    GeneralRegister myRegisters[8];

    // Send everything to the CPU for processing
    CPU myCPU(&myFlags, myRegisters);

      // Create the Runner
    Runner runner(&myCPU);

    // Load and run the assembly program
    char fileName[100];
    cout << "Enter asm file name: ";
    cin >> fileName;

    if (runner.loadProgram(fileName))
    {
        runner.run();
        myCPU.dump();
    }

    return 0;
}

// function defintion of helper function
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
        default:
            break;    
    }
    return false;
}

// function definition pf Runner methods
bool Runner::isRegister(string text)
{ return text.length() == 2 && text[0] == 'R' && text[1] >= '0' && text[1] <= '7'; }

int Runner::getRegisterNumber(string text)
{
    if (isRegister(text)) return text[1] - '0';
    return -1;
}

bool Runner::isBracket(string text)
{ return text.length() >= 3 && text[0] == '[' && text[text.length() - 1] == ']'; }

string Runner::removeBracket(string text)
{
    if (isBracket(text)) return text.substr(1, text.length() - 2);
    return text;
}

int Runner::getNumber(string text)
{
    stringstream ss(text);
    int value = 0;
    ss >> value;
    return value;
}

Flags Runner::getFlagName(string text)
{
    if (text == "OF") return Flags::OF;
    else if (text == "UF") return Flags::UF;
    else if (text == "CF") return Flags::CF;
    else if (text == "ZF") return Flags::ZF;
    else return Flags::COUNT;
}

Opcode Runner::getOpcodeName(string text)
{
    if (text == "ADD") return Opcode::ADD;
    else if (text == "SUB") return Opcode::SUB;
    else if (text == "MUL") return Opcode::MUL;
    else if (text == "DIV") return Opcode::DIV;
    else if (text == "INC") return Opcode::INC;
    else if (text == "DEC") return Opcode::DEC;
    else if (text == "INPUT") return Opcode::INPUT;
    else if (text == "DISPLAY") return Opcode::DISPLAY;
    else if (text == "MOV") return Opcode::MOV;
    else if (text == "LOAD") return Opcode::LOAD;
    else if (text == "STORE") return Opcode::STORE;
    else if (text == "RESET") return Opcode::RESET;
    else if (text == "PUSH") return Opcode::PUSH;
    else if (text == "POP") return Opcode::POP;
    else if (text == "SHL") return Opcode::SHL;
    else if (text == "SHR") return Opcode::SHR;
    else if (text == "ROL") return Opcode::ROL;
    else if (text == "ROR") return Opcode::ROR;
    else return Opcode::COUNT;
}
    
Instruction* Runner::loadArithmeticInst(Opcode opc, string opr1, string opr2)
{
    if (opc == Opcode::INC || opc == Opcode::DEC)
    { return new ArithmeticInstruction(*cpu, opc, getRegisterNumber(opr1)); }
    bool isReg = isRegister(opr2);
    int operand2 = isReg ? getRegisterNumber(opr2) : getNumber(opr2);
    return new ArithmeticInstruction(*cpu, opc, getRegisterNumber(opr1), operand2, isReg);
}

Instruction* Runner::loadIOInst(Opcode opc, string opr1)
{ return new IOInstruction(*cpu, opc, getRegisterNumber(opr1)); }

Instruction* Runner::loadShiftInst(Opcode opc, string opr1, string opr2)
{ return new ShiftInstruction(*cpu, opc, getRegisterNumber(opr1), getNumber(opr2)); }

Instruction* Runner::loadDataMovementInst(Opcode opc, string opr1, string opr2)
{
    switch (opc) 
    {
        case (Opcode::MOV) :
            return loadMOVInst(opr1, opr2);
        case (Opcode::LOAD) :
            return loadLOADInst(opr1, opr2);
        case (Opcode::STORE) :
            return loadSTOREInst(opr1, opr2);
        default:
            return NULL;
    }
}

Instruction* Runner::loadMOVInst(string opr1, string opr2)
{
    bool indirect = false, isReg = false;
    int operand2 = 0;

    if (isBracket(opr2))
    {
        string inside = removeBracket(opr2);
        indirect = isRegister(inside);
        isReg = indirect;
        operand2 = indirect ? getRegisterNumber(inside) : getNumber(inside);
    }
    else
    {
        isReg = isRegister(opr2);
        operand2 = isReg ? getRegisterNumber(opr2) : getNumber(opr2);
    }

    return new DataMovementInstruction(*cpu, Opcode::MOV, getRegisterNumber(opr1), operand2, isReg, indirect);
}

Instruction* Runner::loadLOADInst(string opr1, string opr2)
{
    string src = removeBracket(opr2);
    bool isReg = isRegister(src);
    int operand2 = isReg ? getRegisterNumber(src) : getNumber(src);
    return new DataMovementInstruction(*cpu, Opcode::LOAD, getRegisterNumber(opr1), operand2, isReg);
}

Instruction* Runner::loadSTOREInst(string opr1, string opr2)
{
    if (isRegister(opr1))   // STORE R1, 10, STORE R1, R2
    {
        string dest = removeBracket(opr2);
        bool isReg = isRegister(dest);
        int operand2 = isReg ? getRegisterNumber(dest) : getNumber(dest);
        return new DataMovementInstruction(*cpu, Opcode::STORE, getRegisterNumber(opr1), operand2, isReg);
    }
    else    // STORE [10], R1
    {
        int address = getNumber(removeBracket(opr1));
        int reg = getRegisterNumber(opr2);
        return new DataMovementInstruction(*cpu, Opcode::STORE, reg, address, false);
    }
}

Instruction* Runner::loadStackInst(Opcode opc, string opr1)
{ return new StackInstruction(*cpu, opc, getRegisterNumber(opr1)); }

Instruction* Runner::loadResetInst(Opcode opc, string opr1)
{ return new RESETInstruction(*cpu, Opcode::RESET, getFlagName(opr1)); }

Runner::Runner(CPU* c)
    : cpu(c),
    instructionCount(0)
{}

void Runner::cleanLine(char line[])
{
    for (int i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == ',' || line[i] == '\t')
        { line[i] = ' '; }
    }
}

void Runner::parseLine(char line[], ParsedCommand& inst)
{
    cleanLine(line);
    string opcode = "", op1 = "", op2 = "";
    stringstream ss(line);
    ss >> opcode >> op1 >> op2;
    inst = ParsedCommand(opcode, op1, op2);
}

bool Runner::loadProgram(const char fileName[])
{
    ifstream file(fileName);
    if (!file)
    {
        cerr << "Cannot open asm file.\n";
        return false;
    }

    instructionCount = 0;
    char line[100];

    while (file.getline(line, 100))
    {
        ParsedCommand* inst = new ParsedCommand("", "", "");
        parseLine(line, *inst);
        if (inst->getOpcode() == "") 
        { 
            delete inst;
            continue;
        }
        programs.pushBack(inst);
        instructionCount++;
    }

    file.close();
    return true;
}

ExecutionResult Runner::decodeProgram()
{
    for (int i=0; i < Runner::instructionCount; i++)
    {
        Opcode opc = getOpcodeName(programs[i]->getOpcode());
        if (opc == Opcode::COUNT) return ExecutionResult::InvalidInstruction;

        string opr1 = programs[i]->getOperand1();
        string opr2 = programs[i]->getOperand2();
        loadInstruction(opc, opr1, opr2);
    }
    return ExecutionResult::Success;
}

void Runner::loadInstruction(Opcode opc, string opr1, string opr2)
{
    Instruction* inst;
    switch (opc)
    {
        case (Opcode::ADD) : case (Opcode::SUB) : case (Opcode::MUL) : case (Opcode::DIV) : case (Opcode::INC) : case (Opcode::DEC) :
            inst = loadArithmeticInst(opc, opr1, opr2);
            break;
        case (Opcode::INPUT) : case (Opcode::DISPLAY) : 
            inst = loadIOInst(opc, opr1);
            break;
        case (Opcode::SHL) : case (Opcode::SHR) : case (Opcode::ROL) : case (Opcode::ROR) : 
            inst = loadShiftInst(opc, opr1, opr2);
            break;
        case (Opcode::MOV) : case (Opcode::LOAD) : case (Opcode::STORE) :  
            inst = loadDataMovementInst(opc, opr1, opr2);
            break;
        case (Opcode::PUSH) : case (Opcode::POP) : 
            inst = loadStackInst(opc, opr1);
            break;
        case (Opcode::RESET) : 
            inst = loadResetInst(opc, opr1);
            break;
    }
    instructions.pushBack(inst);
}

void Runner::run()
{
    while(cpu->getPC() < instructionCount)
    {
        int pc = cpu->getPC();
        ExecutionResult execResult = instructions[pc]->execute();
        cpu->incrementPC();
        if (!handleExecResult(execResult, pc)) { return; }
    }
}

// function defintion for cpu.dump
void CPU::dump() const
{
    string flagName[] = {"OF", "UF", "CF", "ZF"};
    cout << "#Begin#\n";
    cout << "#Registers#";
    for (int i = 0; i < 8; i++) 
        { cout << setw(4) << setfill('0') << static_cast<int>(getRegister(i)) << "#"; }
    cout << "\n#Flags#";
    for (int i = 0; i < static_cast<int>(Flags::COUNT); i++) 
        { cout << flagName[i] << "#" << static_cast<int>(getFlag(static_cast<Flags>(i))) << "#"; }
    cout << "\n#PC#" << setw(4) << setfill('0') << getPC() << "#";
    cout << "\n#Memory#\n#";
    for (int i = 0; i < 64; i++) 
    {
        int value;
        ExecutionResult e = getMemory(i, value);
        cout << setw(4) << setfill('0') << value << "#";
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
    ExecutionResult execResult;

    if (indirect) 
    {
        int memAdr = cpu.getRegister(operand2);
        execResult = cpu.getMemory(memAdr, value);   
    }
    else value = isReg? cpu.getRegister(operand2) : operand2;

    updateReg(operand1, value);
    return execResult;
}

ExecutionResult DataMovementInstruction::load()
{
    int value;
    int memAdr = isReg? cpu.getRegister(operand2) : operand2;
    ExecutionResult execResult = cpu.getMemory(memAdr, value);
    updateReg(operand1, value);
    return execResult;
}

ExecutionResult DataMovementInstruction::store()
{
    int memAdr = isReg? cpu.getRegister(operand2) : operand2;
    int8_t value = cpu.getRegister(operand1);
    ExecutionResult execResult = cpu.setMemory(memAdr, value);
    return execResult;
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
            return pop(); 
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