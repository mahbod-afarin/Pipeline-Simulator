#include <string>
#include <vector>


/* Types of possible instruction types */
enum InstructionType {
	NOP = 0,	// NOP. Pipeline bubble.
	ADD,		// Add
	SUB,		// Subtract
	MULT,		// Multiply
	DIV,		// Divide
	LW,		// Load word
	SW,		// Store word
	BNEZ		// Branch not equal to zero

};

/* Names of possible instruction types */
const std::string instructionNames[8] = {"*", "ADD", "SUB", "MULT", "DIV", "LW", "SW", "BNEZ"};

enum Stage {
	FETCH = 0,
	DECODE,
	EXEC,
	MEM,
	WB,
	NONE
};

const std::string stageNames[6] = {"FETCH", "DECODE", "EXEC", "MEM", "WB", "NONE"};

/* A Single Register Entry containing register number and register data value */
class Register {
	public:
		Register();
		int dataValue;
		int registerNumber;
		std::string registerName;
};

/* Register file with 16 registers */
extern Register registerFile[16];

class Instruction {
	public:
		Instruction();
		Instruction(std::string);
		InstructionType type; // Type of instruction
		int dest; // Destination register number
		int src1; // Source register number
		int src2; // Source register number
		void printInstruction();
		Stage stage;
};

class Application {
	
	public:
		Application();
		void loadApplication(std::string);
		void printApplication();
		Instruction* getNextInstruction();
		std::vector<Instruction*> instructions;
		int PC; 
};


class PipelineStage {

	public:
		PipelineStage();
		Instruction *inst;
		Stage stageType;
		void clear();
		void addInstruction(Instruction*);
		void printStage();
		void process();
};


class Pipeline {

	public: 
		Pipeline(Application*);
		int cycleTime;
		Application *application;
		PipelineStage pipeline[5];
		void cycle();
		void printPipeline();
		bool done();
		bool hasDependency();
		bool forwarding;
};


