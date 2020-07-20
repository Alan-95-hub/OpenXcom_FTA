#pragma once
/*
 * Copyright 2010-2020 OpenXcom Developers.
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
#include "../Engine/State.h"

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextList;
class Base;


/**
* Covert operation screen that lets the player manage
* all the operations of a base.
*/
class CovertOperationState : public State
{
private:
	Base* _base;
	TextButton* _btnNew, * _btnOk;
	Window* _window;
	Text* _txtTitle, * _txtSoldiersAvailable, * _txtScientistsAvailable, * _txtEngineersAvailable, * _txtOperation, * _txtChances, * _txtProgress;
	TextList* _lstOperations;
public:
	/// Creates the CovertOperation state.
	CovertOperationState(Base* base);
	/// Cleans up the CovertOperation state.
	~CovertOperationState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action* action);
	/// Handler for clicking the New Operation button.
	void btnNewClick(Action* action);
	/// Handler for clicking the CovertOperation list.
	void onSelectOperation(Action* action);
	/// Handler for opening the Current Global Research UI.
	void onCurrentGlobalResearchClick(Action* action);
	/// Fills the CovertOperation list with Base CovertOperation.
	void fillProjectList(size_t scrl);
	/// Updates the CovertOperation list.
	void init() override;
};

class TextEdit;
class RuleCovertOperation;

/**
 * Window which displays list of possible covert operations.
 */
class CovertOperationsListState : public State
{
private:
	Base* _base;
	TextButton* _btnOK;
	Window* _window;
	Text* _txtTitle;
	TextList* _lstOperations;
	size_t _lstScroll;
	void onSelectOperation(Action* action);
	std::vector<RuleCovertOperation*> _operationRules;
public:
	/// Creates the Covert Operations List state.
	CovertOperationsListState(Base* base);
	/// Handler for clicking the OK button.
	void btnOKClick(Action* action);
	/// Fills the ResearchProject list with possible ResearchProjects.
	void fillOperationList();
	/// Initializes the state.
	void init() override;
};

}
