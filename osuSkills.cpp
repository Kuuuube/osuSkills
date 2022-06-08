#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "Slider.h"
#include "filereader.h"
#include "generic.h"
#include "tenacity.h"
#include "stamina.h"
#include "agility.h"
#include "reading.h"
#include "mods.h"
#include "patterns.h"
#include "precision.h"
#include "accuracy.h"
#include "memory.h"
#include "reaction.h"
#include "strains.h"
#include "tweakvars.h"

bool PreprocessMap(Beatmap &beatmap)
{
	if (beatmap.hitObjects.size() < 2)
	{
		return 0;
	}

	PrepareTimingPoints(beatmap);
	ApproximateSliderPoints(beatmap);
	BakeSliderData(beatmap);

	PrepareAimData(beatmap);
	PrepareTapData(beatmap);
	FindPatterns(beatmap);
	AnalyzePatterns(beatmap);
	
	if (beatmap.distances.size() == 0)
		return 0;
	return 1;
}

void CalculateSkills(Beatmap &beatmap)
{
	CalculateReaction(beatmap, HasMod(beatmap, HD));
	CalculateStamina(beatmap);
	CalculateTenacity(beatmap);
	bool agilityV2 = false;
	if (agilityV2)
		CalculateAgilityStrains(beatmap);  // calculates precision as well. Might seperate that later
	else
		CalculateAgility(beatmap);
	CalculatePrecision(beatmap, HasMod(beatmap, HD));
	CalculateAccuracy(beatmap);
	if (HasMod(beatmap, FL))
		CalculateMemory(beatmap);
}

void PrintResults(Beatmap &beatmap)
{
	ofstream OutputFile("output.txt", std::ios_base::app);
	OutputFile << "Stamina: " << beatmap.skills.stamina << " Tenacity: " << beatmap.skills.tenacity << " Agility: " << beatmap.skills.agility << " Accuracy: " << beatmap.skills.accuracy << " Precision: " << beatmap.skills.precision << " Reaction: " << beatmap.skills.reaction << " Memory: " << beatmap.skills.memory << " Mods:" << beatmap.modsString << ", MD5: " << endl;
	OutputFile.close();
}

bool ProcessFile(std::string filepath)
{
	std::vector<std::string> tokens;
	FileReader::tokenize(filepath, tokens, "\"");
	int mods = 0;
	if (tokens.size() >= 2 && tokens[2].length()) // there are some mods here probably!
	{
		std::vector<std::string> tokensMods;
		FileReader::tokenize(tokens[2], tokensMods, " ");
		if (tokensMods.size())
		{
			for (auto mod : tokensMods)
			{
				if (!mod.compare(0, mod.length(), "EZ"))
					mods += EZ;
				if (!mod.compare(0, mod.length(), "HT"))
					mods += HT;
				if (!mod.compare(0, mod.length(), "HR"))
					mods += HR;
				if (!mod.compare(0, mod.length(), "DT"))
					mods += DT;
				if (!mod.compare(0, mod.length(), "HD"))
					mods += HD;
				if (!mod.compare(0, mod.length(), "FL"))
					mods += FL;
			}
		}
	}

	if(tokens.size() >= 2)
		filepath = tokens[1];

	std::ifstream beatmapFile(filepath);
	if (beatmapFile.is_open())
	{
		Beatmap beatmap;
		if (!FileReader::ParseBeatmap(beatmapFile, beatmap))
		{
			return 0;
		}
		beatmapFile.close();
		if (mods)
		{
			beatmap.modsString = tokens[2];
			ApplyMods(beatmap, mods);
		}
		else
		{
			beatmap.modsString = " NM";
		}
		PreprocessMap(beatmap);
		CalculateAimStrains(beatmap);
		CalculateTapStrains(beatmap);
		CalculateSkills(beatmap);
		PrintResults(beatmap);
		return 1;
	}
	else
	{
		return 0;
	}
}

int main(int argc, char** argv)
{
	while (1)
	{
		LoadFormulaVars();

		std::string line;
		getline(std::cin, line);
		if (!line.length())
			break;
		ProcessFile(line);
	}
		
	return 0;
}