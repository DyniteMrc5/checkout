#include "deal.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

std::string Deal::name() const
{
	return iName;
}

std::string& Deal::name()
{
	return iName;
}

std::shared_ptr<Deal> Deal::deserialise(std::string aData)
{
	auto spaceIter = std::find_if(aData.begin(), aData.end(), [](char aChar) { return aChar == ' '; });
	std::string type(aData.begin(), spaceIter);
	DealType dealtype = (DealType)std::stoi(type);
	switch (dealtype)
	{
		case EBuyAofXGetBofYFZ:
		{
			BuyAofXGetBofYForZ* deal = BuyAofXGetBofYForZ::deserialise(aData);
			return std::shared_ptr<Deal>(deal);
		}
		case EBuyInSetOfXCheapestFree:
		{
			BuyInSetOfXCheapestFree* deal = BuyInSetOfXCheapestFree::deserialise(aData);
			return std::shared_ptr<Deal>(deal);
		}
	}

	throw "Invalid deserialse data";
}


bool SmartDeal::selectsOn(const Item & aItem) const
{
	for (DealSelector* ds : iSelectors.selectors())
	{
		SelectionSelector* selector = std::get<0>(ds->iSelector);
		std::vector<Item> itemList{ aItem };
		if (selector->select(itemList).size())
		{
			return true;
		}
	}
	return false;
}

bool SmartDeal::targets(const Item & aItem) const
{
	for (DealSelector* ds : iSelectors.selectors())
	{
		TargetSelector* selector = std::get<1>(ds->iSelector);
		std::vector<Item> itemList{ aItem };
		if (selector->select(itemList).size())
		{
			return true;
		}
	}
	return false;
}

std::string SmartDeal::serialise()
{
	return std::string();
}

SmartDeal* SmartDeal::deserialise(std::string aData)
{
	return nullptr;
}

std::vector<std::pair<Item, int>> SmartDeal::evaluate(std::vector<Item>& aInput) const
{
	//*******
	int printaInput = 1;
	printaInput++;
	//*******

	std::vector<std::pair<Item, int>> result{};

	std::vector<Item> input = aInput;

	bool found = true;

	for (DealSelector* ds : iSelectors.selectors())
	{
		DealSelectorSelectTargetPrice selectorPair = ds->iSelector;

		// Does this deal qualify given this input?
		SelectionSelector* selector = std::get<0>(selectorPair);
		std::vector<Item> selected = selector->select(input);
		int numSelected = selected.size();

		if (numSelected == 0)
		{
			if (ds->strict())
			{
				result.clear();
				return result;
			}
			else {
				continue; // optional DS - continue
			}
		}

		// find target item(s)
		TargetSelector* targetSelector = std::get<1>(selectorPair);
		std::vector<Item> targets = targetSelector->select(input);

		int numTargets = targets.size();

		if (numTargets == 0)
		{
			if (ds->strict())
			{
				result.clear();
				return result;
			} else {
				continue; // optional DS - continue
			}
		}

		// If we have any selected which are also in targets, remove from selected.
		// ((Item)selected[..]).iId == ((Item)targets[..]).iId
		for (Item& item : targets)
		{
			auto findItemInSelected = std::find(selected.begin(), selected.end(), item);
			if (findItemInSelected != selected.end())
			{
				selected.erase(findItemInSelected);
			}
		}

		// Add all targets to result
		for (Item& item : targets)
		{
			int unitPrice = std::get<2>(selectorPair);
			result.push_back(std::make_pair(item, unitPrice));
			//remove targets from input items (deals cannot be used in conjunction)
			input.erase(std::find(input.begin(), input.end(), item));
		}
		// Add all selected to result
		for (Item& item : selected)
		{
			int unitPrice = std::get<2>(selectorPair);

			result.push_back(std::make_pair(item, item.iUnitPrice));

			//remove selected items from input items (deals cannot be used in conjunction)
			auto find = std::find(input.begin(), input.end(), item);
			if (find != input.end())
			{
				input.erase(find);
			}
		}
	}

	return result;
}