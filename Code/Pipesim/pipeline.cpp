#include "pipeline.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>

Register registerFile[16];

Register::Register(void)
{

	dataValue = 0;
	registerNumber = -1;
	registerName = "";
}

Instruction::Instruction(void)
{

	type = NOP;
	dest = -1;
	src1 = -1;
	src2 = -1;
	stage = NONE;
}

Instruction::Instruction(std::string newInst)
{

	std::string buf;
	std::stringstream ss(newInst);
	std::vector<std::string> tokens;

	while (ss >> buf)
	{
		tokens.push_back(buf);
	}

	if (tokens[0] == "ADD")
		type = ADD;
	else if (tokens[0] == "SUB")
		type = SUB;
	else if (tokens[0] == "MULT")
		type = MULT;
	else if (tokens[0] == "DIV")
		type = DIV;
	else if (tokens[0] == "LW")
		type = LW;
	else if (tokens[0] == "SW")
		type = SW;
	else if (tokens[0] == "BNEZ")
		type = BNEZ;
	else
		type = NOP;

	dest = -1;
	src1 = -1;
	src2 = -1;

	if (tokens.size() > 1)
	{
		dest = atoi(tokens[1].erase(0, 1).c_str());
	}
	if (tokens.size() > 2)
	{
		src1 = atoi(tokens[2].erase(0, 1).c_str());
	}
	if (tokens.size() > 3)
	{
		src2 = atoi(tokens[3].erase(0, 1).c_str());
	}

	// Store and BNEZ has 2 source operands and no destination operand
	if (type == SW || type == BNEZ)
	{
		src2 = src1;
		src1 = dest;
		dest = -1;
	}

	stage = NONE;
}

Application::Application(void)
{

	PC = 0;
}

void Application::loadApplication(std::string fileName)
{

	std::string sLine = "";
	Instruction *newInstruction;
	std::ifstream infile;
	infile.open(fileName.c_str(), std::ifstream::in);

	if (!infile)
	{
		std::cout << "Failed to open file " << fileName << std::endl;
		return;
	}

	while (!infile.eof())
	{
		getline(infile, sLine);
		if (sLine.empty())
			break;
		newInstruction = new Instruction(sLine);
		instructions.push_back(newInstruction);
	}

	infile.close();
	std::cout << "Read file completed!!" << std::endl;

	printApplication();
}

void Application::printApplication(void)
{

	std::cout << "Printing Application: " << std::endl;
	std::vector<Instruction *>::iterator it;
	for (it = instructions.begin(); it < instructions.end(); it++)
	{

		(*it)->printInstruction();
		std::cout << std::endl;
	}
}

Instruction *Application::getNextInstruction()
{

	Instruction *nextInst = NULL;

	if (PC < instructions.size())
	{
		nextInst = instructions[PC];
		PC += 1;
	}

	if (nextInst == NULL)
		nextInst = new Instruction();

	return nextInst;
}

PipelineStage::PipelineStage(void)
{
	inst = new Instruction();
	stageType = NONE;
}

void PipelineStage::clear()
{

	inst = NULL;
}

void PipelineStage::process()
{

	// Functionally simulate pipeline stage
	// Since this simulator only models timing, this function currently does nothing
	switch (stageType)
	{
	case FETCH: // Fetch instruction. PC+4
		break;
	case DECODE: // Fetch register operands
		break;
	case EXEC: // Perform ALU operations
		break;
	case MEM: // Load/Store from/to memory
		break;
	case WB: // Writeback result operand to register
		break;
	default:
		break;
	}
}

void PipelineStage::addInstruction(Instruction *newInst)
{

	inst = newInst;
	inst->stage = stageType;
}

Pipeline::Pipeline(Application *app)
{

	pipeline[FETCH].stageType = FETCH;
	pipeline[DECODE].stageType = DECODE;
	pipeline[EXEC].stageType = EXEC;
	pipeline[MEM].stageType = MEM;
	pipeline[WB].stageType = WB;
	cycleTime = 0;

	printPipeline();

	application = app;

	forwarding = false;
}

bool Pipeline::hasDependency(void)
{
	if (forwarding) //adding forwading 
	{
		if (pipeline[DECODE].inst->type == NOP)
			return false;

		for (int i = EXEC; i < WB; i++)
		{

			if (pipeline[i].inst == NULL)
				continue;

			if (pipeline[i].inst->type == NOP)
				continue;

			if (pipeline[EXEC].inst->type == LW)
			{
				if ((pipeline[i].inst->dest != -1) &&
					(pipeline[i].inst->dest == pipeline[DECODE].inst->src1 ||
					 pipeline[i].inst->dest == pipeline[DECODE].inst->src2))
				{
					return true;
				}
			}
		}

		return false;
	}
	else
	{

		if (pipeline[DECODE].inst->type == NOP)
			return false;

		// Checks if dependency exist between Decode stage and Exec, Mem stage
		// We assume the register file can read/write in the same cycle so no data dependency exist with RAW dependency if an instruction is in Decode and WB.
		for (int i = EXEC; i < WB; i++)
		{

			if (pipeline[i].inst == NULL)
				continue;

			if (pipeline[i].inst->type == NOP)
				continue;

			if ((pipeline[i].inst->dest != -1) &&
				(pipeline[i].inst->dest == pipeline[DECODE].inst->src1 ||
				 pipeline[i].inst->dest == pipeline[DECODE].inst->src2))
			{
				return true;
			}
		}

		return false;
	}
}

void Pipeline::cycle(void)
{

	cycleTime += 1;
	// Check for data hazards
	// NOTE: Technically, data hazards are detected in the Decode stage. If a data hazard is detected, at the end of the cycle we write 0's (NOP) to the pipeline register so that a NOP will be generated in the EXEC stage in the next cycle.
	// Doing the check here does a dependency check on the instructions in the previous cycle (we haven't advanced the instructions in the pipeline yet). If a dependency exist in the previous cycle, we stall the pipeline in this cycle.
	bool dependencyDetected = hasDependency();

	// WRITEBACK STAGE
	// Mem -> WB Pipeline register
	pipeline[WB].addInstruction(pipeline[MEM].inst);

	// Writeback
	pipeline[WB].process();

	// MEM STAGE
	// Exec -> Mem Pipeline register
	pipeline[MEM].addInstruction(pipeline[EXEC].inst);

	// Mem
	pipeline[MEM].process();

	// EXEC STAGE
	// Decode -> Exec Pipeline register
	// If dependency detected, stall by inserting NOP instruction
	if (!dependencyDetected)
		pipeline[EXEC].addInstruction(pipeline[DECODE].inst);
	else
		pipeline[EXEC].addInstruction(new Instruction());

	// Exec
	pipeline[EXEC].process();

	// DECODE STAGE
	// Fetch -> Decode Pipeline register
	if (!dependencyDetected)
		pipeline[DECODE].addInstruction(pipeline[FETCH].inst);

	// Decode
	pipeline[DECODE].process();

	// FETCH STAGE
	// Fetch
	if (!dependencyDetected)
	{
		pipeline[FETCH].addInstruction(application->getNextInstruction());
		pipeline[FETCH].process();
	}
}

bool Pipeline::done()
{

	for (int i = 0; i < 5; i++)
	{

		if (pipeline[i].inst->type != NOP)
			return false;
	}

	return true;
}

void Pipeline::printPipeline(void)
{

	if (cycleTime == 0)
		std::cout << "Cycle"
				  << "\tIF"
				  << "\t\tID"
				  << "\t\tEXEC"
				  << "\t\tMEM"
				  << "\t\tWB" << std::endl;
	std::cout << cycleTime;
	for (int i = 0; i < 5; i++)
	{

		pipeline[i].printStage();
	}
	std::cout << std::endl;
}

void PipelineStage::printStage(void)
{

	std::cout << "\t";
	inst->printInstruction();
}

void Instruction::printInstruction(void)
{
	if (type == NOP)
		std::cout << instructionNames[type] << "         ";
	else if (type == SW || type == BNEZ)
		std::cout << instructionNames[type] << " r" << src1 << " r" << src2;
	else if (type == LW)
		std::cout << instructionNames[type] << " r" << dest << " r" << src1;
	else
		std::cout << instructionNames[type] << " r" << dest << " r" << src1 << " r" << src2;
}
