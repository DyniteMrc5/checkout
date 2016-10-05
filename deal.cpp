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
			BuyAofXGetBofYFZ* deal = BuyAofXGetBofYFZ::deserialise(aData);
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
		std::vector<Item> selected = std::get<0>(selectorPair)->select(input);
		int numSelected = selected.size();

		if (numSelected == 0)
		{
			found = false;
		}

		if (ds->strict() && !found)
		{
			result.clear();
			return result;
		}
		else {

			// find target item(s)
			std::vector<Item> targets = std::get<1>(selectorPair)->select(input);

			int numTargets = targets.size();
			if (numTargets == 0)
			{
				result.clear();
				return result;
			}

			std::set<Item> targetted{};

			for (Item& item : targets)
			{
				int unitPrice = std::get<2>(selectorPair);
				result.push_back(std::make_pair(item, unitPrice));
				//remove targets from input items (deals cannot be used in conjunction)
				input.erase(std::find(input.begin(), input.end(), item));
				targetted.insert(item);
			}
			for (Item& item : selected)
			{
				int unitPrice = std::get<2>(selectorPair);

				// If select and target is different, include both/all in output
				// (If they're the same item e.g B1G1Free, don't double include)
				if (targetted.count(item) == 0)
				{
					result.push_back(std::make_pair(item, item.iUnitPrice));
				}
				//remove selected items from input items (deals cannot be used in conjunction)
				auto find = std::find(input.begin(), input.end(), item);
				if (find != input.end())
				{
					input.erase(find);
				}
			}
		}
	}

	return result;
}