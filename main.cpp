#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <cstring>
#include <iomanip> // for setw and fill
#include <cstdio> // for sscanf
#include <sstream>
using namespace std;

// ==============================================
// MADE BY ZHEN LONG
// ==============================================
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

// ==============================================
// MADE BY UMAR ZAID
// ==============================================

// Base class of register (Encapsulates an 8-bit signed value and flag update logic) 
class Register
{
protected:
    int8_t value; // value of the 8-bit itself

public:
    // constructor
    Register() : value()
    {}

    // sets the value of the current register into the parameter that is given 
    void setValue(int8_t val)
    {
        value = val;
    }

    // outputs the current value that is stored inside the register
    int8_t getValue() const
    {
        return value;
    }
};


// Basically a container for the R0-R7 registers, we initialized all 8 registers using this.   (inheritance from Register)
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

// ==============================================
// MADE BY KAR FUNG
// ==============================================
// registers boolean values based on CF OF UF ZF flags
class FlagRegister 
{
private:
    bool cf; // Carry Flag
    bool zf; // Zero Flag
    bool uf; // Underflow Flag
    bool of; // Overflow Flag

public:
    FlagRegister() // this constructor function is used reset state by overwriting them
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

    // Core validation method to check arithmetic bounds it updates the flags to true or false based on the condition
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
            cf = (result >= 256); 
        }
    }
};

// ==============================================
// MADE BY UMAR ZAID
// ==============================================

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
        // changes the value inside the address.
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
        // outputs the value inside the address
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

    int8_t stackStorage[8]; // The 8-byte system stack managed internally
    int si=0;               // Stack Index (SI) register starting at 0

public:
    // ==============================================
    // MADE BY ZHEN LONG
    // ==============================================
    CPU(FlagRegister* flag, GeneralRegister* reg) : flags(flag), registers(reg) {}

    int8_t getMemory(int memAdr) const { return memory.read(memAdr); }
    bool getFlag(Flags flag) const { return flags->getFlag(flag); }
    int8_t getRegister(int regNum) const { return registers[regNum].getValue(); }
    int getPC() const { return pc; }

    void incrementPC() { pc++; }
    void resetPC() { pc = 0; }

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

// ==============================================
// MADE BY AL-SAKKAF
// ==============================================
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

// ==============================================
// MADE BY ZHEN LONG
// ==============================================
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

// ==============================================
// MADE BY ALSAKKAF
// ==============================================
class Runner
{
private:
    CPU* cpu;
    ParsedCommand* programs;
    int instructionCount;

    bool isRegister(string text)
    {
        return text.length() == 2 && text[0] == 'R' && text[1] >= '0' && text[1] <= '7';
    }

    int getRegisterNumber(string text)
    {
        if (isRegister(text)) return text[1] - '0';
        return -1;
    }

    bool isBracket(string text)
    {
        return text.length() >= 3 && text[0] == '[' && text[text.length() - 1] == ']';
    }

    string removeBracket(string text)
    {
        if (isBracket(text)) return text.substr(1, text.length() - 2);
        return text;
    }

    int getNumber(string text)
    {
        stringstream ss(text);
        int value = 0;
        ss >> value;
        return value;
    }

    Flags getFlagName(string text)
    {
        if (text == "OF") return Flags::OF;
        if (text == "UF") return Flags::UF;
        if (text == "CF") return Flags::CF;
        return Flags::ZF;
    }

    Opcode getOpcodeName(string text)
    {
        if (text == "ADD") return Opcode::ADD;
        if (text == "SUB") return Opcode::SUB;
        if (text == "MUL") return Opcode::MUL;
        if (text == "DIV") return Opcode::DIV;
        if (text == "INC") return Opcode::INC;
        if (text == "DEC") return Opcode::DEC;
        if (text == "INPUT") return Opcode::INPUT;
        if (text == "DISPLAY") return Opcode::DISPLAY;
        if (text == "MOV") return Opcode::MOV;
        if (text == "LOAD") return Opcode::LOAD;
        if (text == "STORE") return Opcode::STORE;
        if (text == "RESET") return Opcode::RESET;
        if (text == "PUSH") return Opcode::PUSH;
        if (text == "POP") return Opcode::POP;
        if (text == "SHL") return Opcode::SHL;
        if (text == "SHR") return Opcode::SHR;
        if (text == "ROL") return Opcode::ROL;
        return Opcode::ROR;
    }

public:
    Runner()
    {
        cpu = NULL;
        programs = NULL;
        instructionCount = 0;
    }

    Runner(CPU* c)
    {
        cpu = c;
        programs = NULL;
        instructionCount = 0;
    }

    void setCPU(CPU* c)
    {
        cpu = c;
    }

    void cleanLine(char line[])
    {
        for (int i = 0; line[i] != '\0'; i++)
        {
            if (line[i] == ',' || line[i] == '\t')
                line[i] = ' ';
        }
    }

    void parseLine(char line[], ParsedCommand& inst)
    {
        cleanLine(line);

        string opcode = "", op1 = "", op2 = "";
        stringstream ss(line);

        ss >> opcode >> op1 >> op2;
        inst = ParsedCommand(opcode, op1, op2);
    }

    bool loadProgram(const char fileName[])
    {
        ifstream file(fileName);
        if (!file)
        {
            cerr << "Cannot open asm file.\n";
            return false;
        }

        programs = new ParsedCommand[100];
        instructionCount = 0;
        char line[100];

        while (file.getline(line, 100))
        {
            ParsedCommand inst;
            parseLine(line, inst);

            if (inst.getOpcode() == "") continue;

            programs[instructionCount] = inst;
            instructionCount++;
        }

        file.close();
        if (cpu != NULL) cpu->resetPC();
        return true;
    }

    void displayInstruction(ParsedCommand inst)
    {
        cout << inst.getOpcode();
        if (inst.getOperand1() != "") cout << " " << inst.getOperand1();
        if (inst.getOperand2() != "") cout << ", " << inst.getOperand2();
        cout << endl;
    }

    void run()
    {
        if (cpu == NULL) return;

        for (int i = 0; i < instructionCount; i++)
        {
            ParsedCommand cmd = programs[i];
            string op = cmd.getOpcode();
            string a = cmd.getOperand1();
            string b = cmd.getOperand2();

            Instruction* instruction = NULL;
            ExecutionResult result = ExecutionResult::Success;

            if (op == "INPUT" || op == "DISPLAY")
            {
                instruction = new IOInstruction(*cpu, getOpcodeName(op), getRegisterNumber(a));
            }
            else if (op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV")
            {
                bool isReg = isRegister(b);
                int op2 = isReg ? getRegisterNumber(b) : getNumber(b);
                instruction = new ArithmeticInstruction(*cpu, getOpcodeName(op), getRegisterNumber(a), op2, isReg);
            }
            else if (op == "INC" || op == "DEC")
            {
                instruction = new ArithmeticInstruction(*cpu, getOpcodeName(op), getRegisterNumber(a));
            }
            else if (op == "SHL" || op == "SHR" || op == "ROL" || op == "ROR")
            {
                instruction = new ShiftInstruction(*cpu, getOpcodeName(op), getRegisterNumber(a), getNumber(b));
            }
            else if (op == "MOV")
            {
                bool indirect = false;
                bool isReg = false;
                int op2 = 0;

                if (isBracket(b))
                {
                    string inside = removeBracket(b);
                    indirect = isRegister(inside);
                    isReg = indirect;
                    op2 = indirect ? getRegisterNumber(inside) : getNumber(inside);
                }
                else
                {
                    isReg = isRegister(b);
                    op2 = isReg ? getRegisterNumber(b) : getNumber(b);
                }

                instruction = new DataMovementInstruction(*cpu, Opcode::MOV, getRegisterNumber(a), op2, isReg, indirect);
            }
            else if (op == "LOAD")
            {
                string src = removeBracket(b);
                bool isReg = isRegister(src);
                int op2 = isReg ? getRegisterNumber(src) : getNumber(src);
                instruction = new DataMovementInstruction(*cpu, Opcode::LOAD, getRegisterNumber(a), op2, isReg);
            }
            else if (op == "STORE")
            {
                if (isRegister(a))
                {
                    string dest = removeBracket(b);
                    bool isReg = isRegister(dest);
                    int op2 = isReg ? getRegisterNumber(dest) : getNumber(dest);
                    instruction = new DataMovementInstruction(*cpu, Opcode::STORE, getRegisterNumber(a), op2, isReg);
                }
                else
                {
                    int address = getNumber(removeBracket(a));
                    int reg = getRegisterNumber(b);
                    instruction = new DataMovementInstruction(*cpu, Opcode::STORE, reg, address, false);
                }
            }
            else if (op == "PUSH" || op == "POP")
            {
                instruction = new StackInstruction(*cpu, getOpcodeName(op), getRegisterNumber(a));
            }
            else if (op == "RESET")
            {
                instruction = new RESETInstruction(*cpu, Opcode::RESET, getFlagName(a));
            }
            else
            {
                result = ExecutionResult::InvalidInstruction;
            }

            if (instruction != NULL)
            {
                result = instruction->execute();
                delete instruction;
            }

            if (!handleExecResult(result, cpu->getPC())) return;
            cpu->incrementPC();
        }
    }

    int getInstructionCount()
    {
        return instructionCount;
    }
};

// ==============================================
// MADE BY UMAR ZAID
// ==============================================

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

// ==============================================
// MADE BY AL SAKKAF
// ==============================================
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

// ==============================================
// MADE BY ZHEN LONG
// ==============================================
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