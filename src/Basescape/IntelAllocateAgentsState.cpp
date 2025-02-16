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
#include "IntelAllocateAgentsState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/Options.h"
#include "../Engine/Unicode.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleSoldier.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/BasePrisoner.h"
#include "SoldierInfoStateFtA.h"
#include "../Mod/RuleIntelProject.h"
#include "../Savegame/IntelProject.h"
#include <algorithm>

namespace OpenXcom
{

/**
 * Initializes all the elements in the Soldiers screen.
 * @param base Pointer to the base to get info from.
 * @param operation Pointer to starting (not committed) covert operation.
 */
IntelAllocateAgentsState::IntelAllocateAgentsState(Base *base, IntelProject* project)
	: _base(base), _project(project), _otherCraftColor(0), _origAgentOrder(*_base->getSoldiers()), _dynGetter(NULL)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(148, 16, 164, 176);
	//_btnInfo = new TextButton(42, 16, 270, 8);
	_txtTitle = new Text(300, 17, 16, 7);
	_txtName = new Text(114, 9, 16, 32);
	_txtAssignment = new Text(84, 9, 122, 32);
	_cbxSortBy = new ComboBox(this, 148, 16, 8, 176, true);
	_lstAgents = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("intelAllocateAgents");

	add(_window, "window", "intelAllocateAgents");
	add(_btnOk, "button", "intelAllocateAgents");
	//add(_btnInfo, "button", "intelAllocateAgents");
	add(_txtTitle, "text", "intelAllocateAgents");
	add(_txtName, "text", "intelAllocateAgents");
	add(_txtAssignment, "text", "intelAllocateAgents");
	add(_lstAgents, "list", "intelAllocateAgents");
	add(_cbxSortBy, "button", "intelAllocateAgents");

	_otherCraftColor = _game->getMod()->getInterface("intelAllocateAgents")->getElement("otherCraft")->color;

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "intelAllocateAgents");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&IntelAllocateAgentsState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&IntelAllocateAgentsState::btnOkClick, Options::keyCancel);

	//_btnInfo->setText(tr("STR_INFO"));
	//_btnInfo->onMouseClick((ActionHandler)&PrisonerAllocateAgentsState::btnInfoClick);

	_txtTitle->setBig();
	_txtTitle->setText(tr(_project->getRules()->getName()));
	_txtTitle->setWordWrap(true);
	_txtTitle->setVerticalAlign(ALIGN_MIDDLE);

	_txtName->setText(tr("STR_NAME_UC"));

	_txtAssignment->setText(tr("STR_ASSIGNMENT"));

	// populate sort options
	std::vector<std::string> sortOptions;
	sortOptions.push_back(tr("STR_ORIGINAL_ORDER"));
	_sortFunctors.push_back(NULL);

#define PUSH_IN(strId, functor)       \
	sortOptions.push_back(tr(strId)); \
	_sortFunctors.push_back(new SortFunctor(_game, functor));

	PUSH_IN("STR_ID", idStat);
	PUSH_IN("STR_NAME_UC", nameStat);

	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::charisma), charismaStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::investigation), investigationStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::deception), deceptionStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::interrogation), interrogationStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::bravery), braveryStat);
	if (_game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements()))
	{
		if (_game->getMod()->isManaFeatureEnabled())
		{
			// "unlock" is checked later
			PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::mana), manaStat);
		}
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::psiStrength), psiStrengthStat);
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::psiSkill), psiSkillStat);
	}

#undef PUSH_IN

	_cbxSortBy->setOptions(sortOptions);
	_cbxSortBy->setSelected(0);
	_cbxSortBy->onChange((ActionHandler)&IntelAllocateAgentsState::cbxSortByChange);
	_cbxSortBy->setText(tr("STR_SORT_BY"));

	_lstAgents->setColumns(2, 106, 174);
	_lstAgents->setAlign(ALIGN_RIGHT, 3);
	_lstAgents->setSelectable(true);
	_lstAgents->setBackground(_window);
	_lstAgents->setMargin(8);
	_lstAgents->onMouseClick((ActionHandler)&IntelAllocateAgentsState::lstAgentsClick, 0);
}

/**
 * cleans up dynamic state
 */
IntelAllocateAgentsState::~IntelAllocateAgentsState()
{
	for (std::vector<SortFunctor *>::iterator it = _sortFunctors.begin();
		 it != _sortFunctors.end(); ++it)
	{
		delete (*it);
	}
}

/**
 * Sorts the soldiers list by the selected criterion
 * @param action Pointer to an action.
 */
void IntelAllocateAgentsState::cbxSortByChange(Action *)
{
	bool ctrlPressed = _game->isCtrlPressed();
	size_t selIdx = _cbxSortBy->getSelected();
	if (selIdx == (size_t)-1)
	{
		return;
	}

	SortFunctor *compFunc = _sortFunctors[selIdx];
	_dynGetter = NULL;
	if (compFunc)
	{
		if (selIdx != 2)
		{
			_dynGetter = compFunc->getGetter();
		}

		// if CTRL is pressed, we only want to show the dynamic column, without actual sorting
		if (!ctrlPressed)
		{
			if (selIdx == 2)
			{
				std::stable_sort(_base->getSoldiers()->begin(), _base->getSoldiers()->end(),
								 [](const Soldier *a, const Soldier *b)
								 {
									 return Unicode::naturalCompare(a->getName(), b->getName());
								 });
			}
			else
			{
				std::stable_sort(_base->getSoldiers()->begin(), _base->getSoldiers()->end(), *compFunc);
			}
			if (_game->isShiftPressed())
			{
				std::reverse(_base->getSoldiers()->begin(), _base->getSoldiers()->end());
			}
		}
	}
	else
	{
		// restore original ordering, ignoring (of course) those
		// soldiers that have been sacked since this state started
		for (std::vector<Soldier *>::const_iterator it = _origAgentOrder.begin();
			 it != _origAgentOrder.end(); ++it)
		{
			std::vector<Soldier *>::iterator soldierIt =
				std::find(_base->getSoldiers()->begin(), _base->getSoldiers()->end(), *it);
			if (soldierIt != _base->getSoldiers()->end())
			{
				Soldier *s = *soldierIt;
				_base->getSoldiers()->erase(soldierIt);
				_base->getSoldiers()->insert(_base->getSoldiers()->end(), s);
			}
		}
	}

	size_t originalScrollPos = _lstAgents->getScroll();
	initList(originalScrollPos);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void IntelAllocateAgentsState::btnOkClick(Action *)
{
	for (auto s : *_base->getSoldiers())
	{
		if (s->getIntelProject() == _project)
		{
			s->clearBaseDuty();
		}
	}

	for (auto s : _assignedAgents)
	{
		s->setIntelProject(_project);
	}
	_game->popState();
}

//void PrisonerAllocateAgentsState::btnInfoClick(Action* action)
//{
//	_game->pushState(new ResearchProjectDetailsState(_base, _planningProject->getResearchRules()));
//}

/**
 * Shows the soldiers in a list at specified offset/scroll.
 */
void IntelAllocateAgentsState::initList(size_t scrl)
{
	int row = 0;
	_lstAgents->clearList();

	_filteredListOfAgents.clear();
	_agentsNumbers.clear();
	_assignedAgents.clear();
	int i = 0;

	for (auto& soldier : *_base->getSoldiers())
	{
		if (soldier->getRoleRank(ROLE_AGENT) > 0)
		{
			_filteredListOfAgents.push_back(soldier);
			_agentsNumbers.push_back(i);
		}
		i++;
	}

	if (_dynGetter != NULL)
	{
		_lstAgents->setColumns(3, 106, 158, 16);
	}
	else
	{
		_lstAgents->setColumns(2, 106, 174);
	}

	auto recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;
	for (std::vector<Soldier *>::iterator s = _filteredListOfAgents.begin(); s != _filteredListOfAgents.end(); ++s)
	{
		std::string duty = (*s)->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree, INTEL);
		if (_dynGetter != NULL)
		{
			// call corresponding getter
			int dynStat = (*_dynGetter)(_game, *s);
			std::ostringstream ss;
			ss << dynStat;
			_lstAgents->addRow(3, (*s)->getName(true, 19).c_str(), duty.c_str(), ss.str().c_str());
		}
		else
		{
			_lstAgents->addRow(2, (*s)->getName(true, 19).c_str(), duty.c_str());
		}

		Uint8 color = _lstAgents->getColor();
		bool matched = false;
		auto agents = *_base->getSoldiers();
		auto iter = std::find(std::begin(agents), std::end(agents), (*s));
		if (iter != std::end(agents))
		{
			matched = true;
		}

		if (matched)
		{
			color = _lstAgents->getSecondaryColor();
			_lstAgents->setCellText(row, 1, tr("STR_ASSIGNED_UC"));
			_assignedAgents.insert(*s);
		}
		else if (isBusy || !isFree)
		{
			color = _otherCraftColor;
		}

		_lstAgents->setRowColor(row, color);
		row++;
	}
	if (scrl)
		_lstAgents->scrollTo(scrl);
	_lstAgents->draw();
}

/**
 * Shows the soldiers in a list.
 */
void IntelAllocateAgentsState::init()
{
	State::init();
	_base->prepareSoldierStatsWithBonuses(); // refresh stats for sorting
	initList(0);
}

/**
 * Shows the selected soldier's info.
 * @param action Pointer to an action.
 */
void IntelAllocateAgentsState::lstAgentsClick(Action *action)
{
	double mx = action->getAbsoluteXMouse();
	if (mx >= _lstAgents->getArrowsLeftEdge() && mx < _lstAgents->getArrowsRightEdge())
	{
		return;
	}
	int row = _lstAgents->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		Soldier *s = _base->getSoldiers()->at(_agentsNumbers.at(row));
		Uint8 color = _lstAgents->getColor();
		bool isBusy = false, isFree = false, matched = false;
		std::string duty = s->getCurrentDuty(_game->getLanguage(), _base->getSumRecoveryPerDay(), isBusy, isFree, INTEL);
		auto agents = *_base->getSoldiers();
		auto iter = std::find(std::begin(agents), std::end(agents), s);
		if (iter != std::end(agents))
		{
			matched = true;
		}
		if (matched)
		{
			_assignedAgents.erase(s);
			if (s->getActivePrisoner())
			{
				if (s->getIntelProject() == _project)
				{
					color = _lstAgents->getColor();
					_lstAgents->setCellText(row, 1, tr("STR_NONE_UC"));
				}
				else
				{
					color = _otherCraftColor;
					_lstAgents->setCellText(row, 1, duty);
				}
			}
			else
			{
				_lstAgents->setCellText(row, 1, duty);
				if (isBusy || !isFree || s->getCraft())
				{
					color = _otherCraftColor;
				}
			}
		}
		else if (s->hasFullHealth() && !isBusy)
		{
			_lstAgents->setCellText(row, 1, tr("STR_ASSIGNED_UC"));
			color = _lstAgents->getSecondaryColor();
			_assignedAgents.insert(s);
		}

		_lstAgents->setRowColor(row, color);
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		_game->pushState(new SoldierInfoStateFtA(_base, _agentsNumbers.at(row)));
	}
}

}
