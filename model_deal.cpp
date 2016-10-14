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
 * BuyAofXGetBofYForZ
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

std::string BuyAofXGetBofYForZ::name() const
{
	return "Buy" + std::to_string(iSelectionCount) + "Of" + std::to_string(iSelectionId) +
		"Get" + std::to_string(iTargetCount) + "Of" + std::to_string(iTargetId) +
		"For" + std::to_string(iTargetUnitPrice) + "UnitPrice";
}

bool BuyAofXGetBofYForZ::selectsOn(const Item & aItem) const
{
	if (iSelectionId == aItem.iId)
		return true;

	return false;
}

bool BuyAofXGetBofYForZ::targets(const Item & aItem) const
{
	if (iTargetId == aItem.iId)
		return true;

	return false;
}

std::vector<std::pair<Item, int>> BuyAofXGetBofYForZ::evaluate(std::vector<Item>& aInput) const
{
	auto result = std::vector<std::pair<Item, int>>();

	// Find Target Items (may/may not exist)
	int targetCount = 0;
	int selectionCount = 0;
	for (Item& item : aInput)
	{
		// We keep going until we've found all of the targets and its selectors
		if (targetCount >= iTargetCount && selectionCount >= iSelectionCount)
		{
			break;
		}

		// Add target items to result with new unit price
		//
		// NB: the "targetCount < iTargetCount" is to cover 2 things:
		//		1) adding too many targets
		//		2) the case when iTargetId == iSelectionId
		//
		//	   In the second case this check prevents a selection item
		//	   being added with iTargetUnitPrice insterad of item.iUnitPrice.
		//
		//	   Selection items get added below.
		if (item.iId == iTargetId && targetCount < iTargetCount)
		{
			result.push_back(std::make_pair(item, iTargetUnitPrice));
			targetCount++;
			// If target id == selection id, then we also increment selection count 
			// as we cannot select this item below if it's already added here.
			if (iTargetId == iSelectionId)
			{
				++selectionCount;
			}
			continue;
		}

		// Add selection items to the result with original target price
		//
		// NB: the "selectionCount < iSelectionCount" is to prevent adding too many selectors
		if (item.iId == iSelectionId && selectionCount < iSelectionCount)
		{
			result.push_back(std::make_pair(item, item.iUnitPrice));
			++selectionCount;
			continue;
		}
	}

	// Did not qualify:
	if (targetCount < iTargetCount || selectionCount < iSelectionCount)
	{
		return std::vector<std::pair<Item, int>>();
	}

	return result;
}

std::string BuyAofXGetBofYForZ::serialise()
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



BuyAofXGetBofYForZ* BuyAofXGetBofYForZ::deserialise(std::string aData)
{
	std::vector<std::string> split = ::split(aData, ' ');
	int data[5];
	for (int i = 1; i < split.size(); ++i)
	{
		data[i - 1] = stoi(split[i]);
	}

	BuyAofXGetBofYForZ* deal = new BuyAofXGetBofYForZ{ data[0], data[1], data[2], data[3], data[4] };
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
