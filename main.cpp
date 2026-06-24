#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

using namespace std;
class CPU;


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

// Handles PUSH and POP operations 
class StackInstruction : public Instruction 
{
private:
    string opCode; // "PUSH" or "POP"
    int regIndex;  // Which register (0-7) to interact with 
    CPU* cpu;      // Reference to the CPU to access stack functions 

public:
    StackInstruction(string op, int regIdx, CPU* c) : opCode(op), regIndex(regIdx), cpu(c) {}

    void execute() override 
    {
        if (opCode == "PUSH") 
        {
            // Get value from register and push it 
            int8_t val = cpu->getRegisterValue(regIndex); 
            cpu->pushToStack(val);
        } 
        else if (opCode == "POP") 
        {
            // Pop value from stack and store it in the register 
            int8_t val = cpu->popFromStack();
            cpu->setRegisterValue(regIndex, val);
            
            // CRITICAL: Update flags because a destination register changed! 
            cpu->getFlags()->updateFlags(val, false, false);
        }
    }
};

// Handles resetting individual flags 
class ResetInstruction : public Instruction 
{
private:
    string flagName; // "CF", "ZF", "OF", or "UF" 
    CPU* cpu;

public:
    ResetInstruction(string flag, CPU* c) : flagName(flag), cpu(c) {}

    void execute() override 
    {
        cpu->getFlags()->resetFlag(flagName); // Manually forces flag to 0 
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
        
    public:
    // --- Hardware Interface Helpers---
        int8_t getRegisterValue(int index) const { return registers[index].getValue(); }
        void setRegisterValue(int index, int8_t val) { registers[index].setValue(val); }
        FlagRegister* getFlags() const { return flags; }
        
    void pushToStack(int8_t val) 
    {
        // Boundary checking: Ensure we don't overflow the 8-byte limit
        if (si >= 8) 
        {
            cout << "System Error: Stack Overflow!" << endl;
            exit(1); 
        }
        stackStorage[si] = val;
        si++; // Incremented when items are pushed 
    }

    int8_t popFromStack() 
    {
        // Boundary check requirement: Any attempt to pop from an empty stack must cause system to stop and crash
        if (si <= 0) 
        {
            cout << "System Error: Attempted to POP from an empty stack. Machine crashing..." << endl;
            exit(1); 
        }
        si--; // Decremented when an item is popped
        return stackStorage[si];
    }
};
// Loads programs, decodes instructions, delegates execution to `CPU`
class Runner
{

};


int main()
{

    return 0;
}

