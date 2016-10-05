#pragma once

#include <string>
#include <functional>

struct Item
{
	Item(int aId, int aUnitPrice, std::string aName) : 
		iId(aId), iUnitPrice(aUnitPrice), iName(aName)
	{};

	bool operator==(const Item& other) 
	{
		return other.iId == iId &&
			other.iUnitPrice == iUnitPrice;
	};


	bool operator==(Item& other)
	{
		return other.iId == iId &&
			other.iUnitPrice == iUnitPrice;
	};

	bool operator<(Item& other)
	{
		return this->iUnitPrice < other.iUnitPrice;
	};

	bool operator<(const Item& other) const
	{
		return this->iUnitPrice < other.iUnitPrice;
	};

	bool operator>(Item& other)
	{
		return this->iUnitPrice > other.iUnitPrice;
	};

	bool operator>(const Item& other) const
	{
		return this->iUnitPrice > other.iUnitPrice;
	};

	inline std::string name()
	{
		return iName;
	};

	int iId;
	int iUnitPrice;
	std::string iName;
};
