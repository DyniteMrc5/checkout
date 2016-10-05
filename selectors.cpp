#include "selectors.h"
#include <algorithm>

// Select a (1) specific item
std::vector<Item> SingleItemSelector::select(std::vector<Item>& aItems)
{
	std::vector<Item> result{};
	for (Item& item : aItems)
	{
		if (item == this->iSelectionItem)
		{
			result.push_back(item);
			break;
		}
	}

	return result;
}

// Select #X of Item-Y
std::vector<Item> CountedSpecificItemSelector::select(std::vector<Item>& aItems)
{
	std::vector<Item> result{};
	int count = 0;
	for (Item& item : aItems)
	{
		if (item == this->iSelectionItem)
		{
			result.push_back(item);
			++count;
			if (count >= this->iSelectionCount)
			{
				break;
			}
		}
	}

	return result;
}

// Select any #X from [a,b,c,...]
std::vector<Item> CountedAnyInSetSelector::select(std::vector<Item>& aItems)
{
	std::vector<Item> result{};

	int count = 0;
	for (Item& i : aItems)
	{
		if (count >= iSelectionCount)
		{
			return result;
		}

		if (iSelectionSet.count(i))
		{
			result.push_back(i);
			count++;
		}
	}
	return result;
}

// Select #X cheapest from [a,b,c,...]
std::vector<Item> CountedCheapestInSetSelector::select(std::vector<Item>& aItems)
{
	std::vector<Item> sorted = aItems;
	std::sort(sorted.begin(), sorted.end());
	return CountedAnyInSetSelector::select(sorted);
}

// Select (1) from [a,b,c,...]
std::vector<Item> SingleInSetSelector::select(std::vector<Item>& aItems)
{
	std::vector<Item> selected = CountedCheapestInSetSelector::select(aItems);
	return selected;
}

std::vector<Item> DealSelector::select(std::vector<Item>& aItems)
{
	return std::vector<Item>();
}
