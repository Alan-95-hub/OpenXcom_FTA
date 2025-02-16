#pragma once
/*
 * Copyright 2010-2022 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{
class Mod;
class RuleResearch;
class RuleEvent;
class RuleSoldier;

/**
* Definition of interrogation rules for prisoner.
*/
class PrisonerInterrogationRules
{
private:
	std::string _requiredResearchName;
	std::vector<std::string> _unlockResearchNames;
	int _baseResistance;
	bool _diesAfter;

	const RuleResearch* _requiredResearch = nullptr;
	std::vector<const RuleResearch*> _unlockResearches;
public:
	PrisonerInterrogationRules();
	~PrisonerInterrogationRules();

	void load(const YAML::Node& node);
	void afterLoad(const Mod* mod);

	int getBaseResistance() { return _baseResistance; }
	bool isDiesAfterInterrogation() { return _diesAfter; }
	const RuleResearch* getReuiredResearch() const { return _requiredResearch; }
	std::vector<const RuleResearch*> getUnlockedResearches() const { return _unlockResearches; }

};

class PrisonerRecruitingRules
{
private:
	std::string _requiredResearchName;
	std::string _spawnedSoldierRuleName;
	int _difficulty, _eventChance;
	std::vector<std::string> _spawnEvents;

	const RuleResearch* _requiredResearch = nullptr;

public:
	PrisonerRecruitingRules();
	~PrisonerRecruitingRules();

	void load(const YAML::Node& node);
	void afterLoad(const Mod* mod);

	int getDifficulty() { return _difficulty; }
	int getEventChance() { return _eventChance; }
	const std::vector<std::string>& getSpawnedEvents() { return _spawnEvents; }
	const RuleResearch* getReuiredResearch() const { return _requiredResearch; }
	std::string &getSpawnedSoldier() { return _spawnedSoldierRuleName; }
};

class PrisonerTortureRules
{
private:
	int _difficulty, _loyaltyChange, _moraleChange, _cooperationChange, _eventChance;
	std::vector<std::string> _spawnEvents;

public:
	PrisonerTortureRules();
	~PrisonerTortureRules();

	void load(const YAML::Node& node);

	int getDifficulty() { return _difficulty; }
	int getLoyalty() { return _loyaltyChange; }
	int getMorale() { return _moraleChange; }
	int getCooperation() { return _cooperationChange; }
	int getEventChance() { return _eventChance; }
	const std::vector<std::string>& getSpawnedEvents() { return _spawnEvents; }
};

class PrisonerContainingRules
{
private:
	std::string _requiredResearchName;
	int _funds, _cooperationChange;

	const RuleResearch* _requiredResearch = nullptr;

public:
	PrisonerContainingRules();
	~PrisonerContainingRules();

	void load(const YAML::Node& node);
	void afterLoad(const Mod* mod);

	int getFunds() { return _funds; }
	int getCooperation() { return _cooperationChange; }
	const RuleResearch* getReuiredResearch() const { return _requiredResearch; }
};

/**
 * Represents a custom Geoscape event.
 * Events are spawned using Event Script ruleset.
 */
class RulePrisoner
{
private:
	std::string _type;
	int _startingCooperation, _damageOverTime;
	PrisonerInterrogationRules* _interrogationRules = nullptr;
	PrisonerRecruitingRules* _recruitingRules = nullptr;
	PrisonerTortureRules* _tortureRules = nullptr;
	PrisonerContainingRules* _containingRules = nullptr;

public:
	/// Creates a blank RulePrisoner.
	RulePrisoner(const std::string &type);
	/// Cleans up the prisoner ruleset.
	~RulePrisoner();
	/// Loads the event definition from YAML.
	void load(const YAML::Node &node);
	void afterLoad(const Mod* mod);

	const std::string &getType() const { return _type; }
	int getStartingCooperation() const { return _startingCooperation; }
	int getDamageOverTime() const { return _damageOverTime; }
	const PrisonerInterrogationRules &getInterrogationRules() const { return *_interrogationRules; }
	const PrisonerRecruitingRules& getRecruitingRules() const { return *_recruitingRules; }
	const PrisonerTortureRules& getTortureRules() const { return *_tortureRules; }
	const PrisonerContainingRules& getContainingRules() const { return *_containingRules; }
};



}
