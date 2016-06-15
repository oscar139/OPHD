#include "StructureTranslator.h"

#include "Constants.h"

std::map<std::string, StructureID> StructureTranslator::_stringToStructureTable;
std::map<StructureID, std::string> StructureTranslator::_structureToStringTable;

StructureID StructureTranslator::translateFromString(const std::string& _s)
{
	if (_stringToStructureTable.empty())
		buildTables();

	if (_stringToStructureTable.find(_s) == _stringToStructureTable.end())
		return SID_NONE;

	return _stringToStructureTable[_s];
}


const std::string& StructureTranslator::translateToString(StructureID _s)
{
	if (_structureToStringTable.empty())
		buildTables();

	// Compiler warning -- this is/will be a bug at some point that will probably manifest as
	// a null pointer or bad access crash (dangling pointer).
	if (_structureToStringTable.find(_s) == _structureToStringTable.end())
		return "";

	return _structureToStringTable[_s];	
}

void StructureTranslator::buildTables()
{
	// String to ID
	_stringToStructureTable[constants::AGRIDOME] = SID_AGRIDOME;
	_stringToStructureTable[constants::AIR_SHAFT] = SID_AIR_SHAFT;
	_stringToStructureTable[constants::CHAP] = SID_CHAP;
	_stringToStructureTable[constants::COMMAND_CENTER] = SID_COMMAND_CENTER;
	_stringToStructureTable[constants::MINE_FACILITY] = SID_MINE_FACILITY;
	_stringToStructureTable[constants::SEED_FACTORY] = SID_SEED_FACTORY;
	_stringToStructureTable[constants::SEED_LANDER] = SID_SEED_LANDER;
	_stringToStructureTable[constants::SEED_POWER] = SID_SEED_POWER;
	_stringToStructureTable[constants::SEED_SMELTER] = SID_SEED_SMELTER;

	_stringToStructureTable[constants::AG_TUBE_INTERSECTION] = SID_TUBE_INTERSECTION;
	_stringToStructureTable[constants::UG_TUBE_INTERSECTION] = SID_TUBE_INTERSECTION;
	_stringToStructureTable[constants::AG_TUBE_RIGHT] = SID_TUBE_RIGHT;
	_stringToStructureTable[constants::UG_TUBE_RIGHT] = SID_TUBE_RIGHT;
	_stringToStructureTable[constants::AG_TUBE_LEFT] = SID_TUBE_LEFT;
	_stringToStructureTable[constants::UG_TUBE_LEFT] = SID_TUBE_LEFT;
	
	// ID To String
	_structureToStringTable[SID_AGRIDOME] = constants::AGRIDOME;
	_structureToStringTable[SID_AIR_SHAFT] = constants::AIR_SHAFT;
	_structureToStringTable[SID_CHAP] = constants::CHAP;
	_structureToStringTable[SID_COMMAND_CENTER] = constants::COMMAND_CENTER;
	_structureToStringTable[SID_MINE_FACILITY] = constants::MINE_FACILITY;
	_structureToStringTable[SID_SEED_FACTORY] = constants::SEED_FACTORY;
	_structureToStringTable[SID_SEED_LANDER] = constants::SEED_LANDER;
	_structureToStringTable[SID_SEED_POWER] = constants::SEED_POWER;
	_structureToStringTable[SID_SEED_SMELTER] = constants::SEED_SMELTER;

	_structureToStringTable[SID_TUBE_INTERSECTION] = constants::AG_TUBE_INTERSECTION;
	_structureToStringTable[SID_TUBE_RIGHT] = constants::AG_TUBE_RIGHT;
	_structureToStringTable[SID_TUBE_LEFT] = constants::AG_TUBE_LEFT;
}