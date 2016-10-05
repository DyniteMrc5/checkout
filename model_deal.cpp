#include "deal.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

/*
 * This file contains the Model Deals, that is,
 * Deals which do not use any of the 'Selector' class structure.
 * 
 * They handle it in their own way (though using the same principle or selectors and targets).
 * There are two types:
 * BuyInSetOfXCheapestFree and
 * BuyAofXGetBofYFZ
 */

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
}

// Split string by delim (char)
std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

std::string BuyInSetOfXCheapestFree::name() const
{
	return "Buy" + std::to_string(iTargetCount) + "GetCheapestFree";
}

std::vector<std::pair<Item, int>> BuyInSetOfXCheapestFree::evaluate(std::vector<Item>& aInput) const
{
	std::vector<std::pair<Item, int>> result;
	std::vector<Item> valid;

	std::vector<Item> sorted = aInput;

	std::sort(sorted.begin(), sorted.end(), [](const Item& item, const Item& other) { return item.iUnitPrice < other.iUnitPrice; });

	// iTargetSet
	for (Item& item : sorted)
	{
		if (valid.size() >= iTargetCount)
		{
			break;
		}

		if (iInputSet.count(item.iId))
		{
			valid.push_back(item);
		}
	}

	if (valid.size() < iTargetCount)
	{
		return result; //empty
	}

	for (auto iter = valid.begin(); iter < valid.begin() + iTargetCount; ++iter)
	{
		// first item set to free / 0
		int unitPrice = (iter == valid.begin()) ? 0 : (*iter).iUnitPrice;
		result.push_back(std::make_pair(*iter, unitPrice));
	}

	return result;
}

bool BuyInSetOfXCheapestFree::selectsOn(const Item & aItem) const
{
	return targets(aItem);
}

bool BuyInSetOfXCheapestFree::targets(const Item & aItem) const
{
	if (iInputSet.count(aItem.iId))
		return true;

	return false;
}

std::string BuyAofXGetBofYFZ::name() const
{
	return "Buy" + std::to_string(iSelectionCount) + "Of" + std::to_string(iSelectionId) +
		"Get" + std::to_string(iTargetCount) + "Of" + std::to_string(iTargetId) +
		"For" + std::to_string(iTargetUnitPrice) + "UnitPrice";
}

bool BuyAofXGetBofYFZ::selectsOn(const Item & aItem) const
{
	if (iSelectionId == aItem.iId)
		return true;

	return false;
}

bool BuyAofXGetBofYFZ::targets(const Item & aItem) const
{
	if (iTargetId == aItem.iId)
		return true;

	return false;
}

std::vector<std::pair<Item, int>> BuyAofXGetBofYFZ::evaluate(std::vector<Item>& aInput) const
{
	// Evaluate Criteria:
	int count = std::count_if(aInput.begin(), aInput.end(), [this](Item& item) { return item.iId == this->iSelectionId; });

	if (count < iSelectionCount)
	{
		//std::cout << ". Found 0 (due to selection)" << std::endl;
		return std::vector<std::pair<Item, int>>();
	}

	auto result = std::vector<std::pair<Item, int>>();

	// Find Target Items (we know exist)
	int targetCount = 0;
	for (Item& item : aInput)
	{
		if (targetCount >= iTargetCount)
		{
			// Special case: iSelectionId == iTargetId and we select more than we target
			//				We need to remove some selections from the population so that
			//				they cannot be used for other deals
			if (iSelectionId == iTargetId && iTargetCount < iSelectionCount)
			{
				result.push_back(std::make_pair(item, item.iUnitPrice));
			}

			break;
		}

		// Add target items to result with new unit price
		if (item.iId == iTargetId)
		{
			result.push_back(std::make_pair(item, iTargetUnitPrice));
			targetCount++;
		}
	}

	/*
	* Add the selection elements to the output, so that they cannot be used in other deals
	*/
	if (iSelectionId != iTargetId)
	{
		int selectionCount = 0;
		auto finditer = std::find_if(aInput.begin(), aInput.end(), [this](Item& item) { return item.iId == this->iSelectionId; });
		while (finditer != aInput.end() && selectionCount < iSelectionCount)
		{
			Item& item = *finditer;
			int unitPrice = item.iUnitPrice;
			result.push_back(std::make_pair(item, unitPrice));
			++selectionCount;
		}
	}

	//std::cout << ". Found " << targetCount << std::endl;

	return result;
}

std::string BuyAofXGetBofYFZ::serialise()
{
	std::string serial;
	serial += std::to_string(((int)EBuyAofXGetBofYFZ)) + " "
		+ std::to_string(iSelectionCount) + " "
		+ std::to_string(iSelectionId) + " "
		+ std::to_string(iTargetCount) + " "
		+ std::to_string(iTargetId) + " "
		+ std::to_string(iTargetUnitPrice);
	return serial;
}



BuyAofXGetBofYFZ* BuyAofXGetBofYFZ::deserialise(std::string aData)
{
	std::vector<std::string> split = ::split(aData, ' ');
	int data[5];
	for (int i = 1; i < split.size(); ++i)
	{
		data[i - 1] = stoi(split[i]);
	}

	BuyAofXGetBofYFZ* deal = new BuyAofXGetBofYFZ{ data[0], data[1], data[2], data[3], data[4] };
	return deal;
}


std::string BuyInSetOfXCheapestFree::serialise()
{
	std::string serial;
	serial += std::to_string(((int)EBuyInSetOfXCheapestFree)) + " "
		+ std::to_string(iTargetCount);

	for (int id : iInputSet)
	{
		serial += " " + std::to_string(id);
	}
	return serial;
}

BuyInSetOfXCheapestFree* BuyInSetOfXCheapestFree::deserialise(std::string aData)
{
	std::vector<std::string> split = ::split(aData, ' ');
	std::set<int> selection;
	int count = stoi(split[1]);
	for (int i = 2; i < split.size(); ++i)
	{
		selection.insert(stoi(split[i]));
	}

	return new BuyInSetOfXCheapestFree{ selection, count };
}

const std::set<int>& BuyInSetOfXCheapestFree::selection() const
{
	return iInputSet;
}

int BuyInSetOfXCheapestFree::targetCount() const
{
	return iTargetCount;
}
