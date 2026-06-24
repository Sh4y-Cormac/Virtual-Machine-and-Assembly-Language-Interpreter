#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

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

// implementation of the register movement
class GeneralRegister : Register
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
class IOInstruction : Instruction
{
    // insert code here
};


// handles bitwise operations
class ShiftInstruction: Instruction
{
    // insert code here
};

// registers boolean values based on CF OF UF ZF flags
class FlagRegister
{
    // insert code here
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
        Memory memory;          // composition: CPU owns memory
        FlagRegister* flags;    // aggregation: CPU owns external flag register
        GeneralRegister registers[8];
        int pc;
};

// Loads programs, decodes instructions, delegates execution to `CPU`
class Runner
{

};


int main()
{

    return 0;
}

