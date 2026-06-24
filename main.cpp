#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <cstring>
#include <cstdio> // for sscanf


using namespace std;


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

// all assembly commands logic go here e.g 'ADD' or 'MOV' (abstract base class)
class Instruction
{
    public:
        virtual ~Instruction()
        {

        }
        virtual void execute() = 0; // pure virtual: causes classes to implement.

};


// Handles 'ADD', 'SUB' and other arithmetic instructions for assembly
class ArithmeticInstruction : public Instruction
{
    public:
        void execute() override
        {
            // implementation goes here!
        }
    
    
};

// handles inputOutput command for assembly instructions
class IOInstruction : public Instruction
{
    public:
    void execute() override
    {
    // insert code here
    }
};


// handles bitwise operations
class ShiftInstruction: public Instruction
{
    public:
    void execute() override
    {
    // insert code here
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
        
    }

    void displayInstruction(ParsedInstruction inst) {
        
    }

    void run() {
        
    }

    int getInstructionCount() {
        return instructionCount;
    }
};
