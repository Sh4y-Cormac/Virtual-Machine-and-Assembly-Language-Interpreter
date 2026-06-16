#include <iostream>
#include <string>

using namespace std;


// Base class of register (Encapsulates an 8-bit signed value and flag update logic) 
class Register
{
    protected:
        int value; // value of the 8-bit itself

    public:

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
class ArithmeticInstruction : Instruction
{   
    // insert code here
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
    //insert code here
};

// Contains registers, memory, PC, executes instructions 
class CPU
{
    private:
        Memory memory;          // composition: CPU owns memory
        FlagRegister* flags;    // aggregation: CPU owns external flag register

};

// Loads programs, decodes instructions, delegates execution to `CPU`
class Runner
{

};


int main()
{

    return 0;
}

