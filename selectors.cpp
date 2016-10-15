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

bool SingleItemSelector::includesItem(const Item & aItem) const
{
	return const_cast<Item&>(aItem) == iSelectionItem;
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

	if (count < iSelectionCount)
	{
		return std::vector<Item> {};
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

	if (count < iSelectionCount)
	{
		return std::vector<Item> {};
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

std::vector<Item> GreedyAnyInSetSelector::select(std::vector<Item>& aItems)
{
	std::vector<Item> result{};

	for (Item& i : aItems)
	{
		if (iSelectionSet.count(i))
		{
			result.push_back(i);
		}
	}
	return result;
}

bool ManyItemSelector::includesItem(const Item & aItem) const
{
	return iSelectionSet.count(aItem);
}
