#pragma once

#include <set>
#include <vector>
#include <tuple>
#include <memory>

#include "item.hpp"
#include "selectors.h"

/*
 * A 'Deal' interface.
 */
class Deal
{
public:
	Deal(std::string aName) : iName(aName) {};
	virtual ~Deal() {};

	virtual std::string name() const;
	std::string& name();

	virtual std::vector<std::pair<Item,int>> evaluate(std::vector<Item>& aInput) const = 0;
	virtual bool selectsOn(const Item& aItem) const = 0;
	virtual bool targets(const Item& aItem) const = 0;
	virtual std::string serialise() = 0;

	static std::shared_ptr<Deal> deserialise(std::string aData);

	std::string iName{ "Default Deal" };
};

class MultiDealSelector;

/*
 * A SmartDeal is a Deal which is able to use 
 * any number of 'sub-deals' (DealSelectors)
 */
class SmartDeal : public Deal
{
public:
	SmartDeal(MultiDealSelector& aSelectors)
		: Deal("SmartDeal Default"), iSelectors(aSelectors)
	{};
	virtual ~SmartDeal() = default;

	virtual std::vector<std::pair<Item, int>> evaluate(std::vector<Item>& aInput) const;
	virtual bool selectsOn(const Item& aItem) const;
	virtual bool targets(const Item& aItem) const;
	virtual std::string serialise();
	static SmartDeal* deserialise(std::string aData);

private:
	MultiDealSelector& iSelectors;
};

// ---- Model Deals ------------------

enum DealType
{
	EBuyInSetOfXCheapestFree = 0,
	EBuyAofXGetBofYFZ = 1
};

class BuyInSetOfXCheapestFree : public Deal
{
public:
	BuyInSetOfXCheapestFree(std::set<int> aInputSet, int aTargetCount)
		: Deal("BuyInSetOfXCheapestFree"), iInputSet(aInputSet), iTargetCount(aTargetCount) {};

	virtual std::string name() const;

	virtual std::vector<std::pair<Item, int>> evaluate(std::vector<Item>& aInput) const;

	virtual bool selectsOn(const Item& aItem) const;
	virtual bool targets(const Item& aItem) const;

	virtual std::string serialise();
	static BuyInSetOfXCheapestFree* deserialise(std::string aData);

	const std::set<int>& selection() const;
	int targetCount() const;
private:
	std::set<int> iInputSet;
	int iTargetCount;
};


// Buy A=|X| items, get B=|Y| for Z unit price (of Y). e.g.For each #A (equal) items of X, get #K of Y items for Z unit price
// Buy A=|X| items, get B=|X| for Z unit price (of X). e.g Buy 1/2/2 get 1/2 free
// Buy A=|X| items, get A=|X| for Z unit price (of X). e.g Buy 2, Pay 2/0.75.
class BuyAofXGetBofYFZ : public Deal
{
public:
	BuyAofXGetBofYFZ(int aSelectionCount, int aSelectionId, int aTargetCount, int aTargetId, int aTargetUnitPrice) :
		Deal("BuyAofXGetBofYFZ"), iSelectionCount(aSelectionCount), iSelectionId(aSelectionId), iTargetCount(aTargetCount), iTargetId(aTargetId), iTargetUnitPrice(aTargetUnitPrice) 
	{
	};

	virtual std::string name() const;

	virtual bool selectsOn(const Item& aItem) const;
	virtual bool targets(const Item& aItem) const;

	virtual std::vector<std::pair<Item, int>> evaluate(std::vector<Item>& aInput) const;
	//friend bool operator <(const BuyAofXGetBofYFZ& lhs, const BuyAofXGetBofYFZ& rhs) { return &lhs < &rhs; };

	virtual std::string serialise();
	static BuyAofXGetBofYFZ* deserialise(std::string aData);

	inline int selectionCount() { return iSelectionCount; }
	inline int selectionId() { return iSelectionId; }
	inline int targetCount() { return iTargetCount; }
	inline int targetId() { return iTargetId; }
	inline int targetUnitPrice() { return iTargetUnitPrice; }

private:
	int iSelectionCount;	//A
	int iSelectionId;		//X

	int iTargetCount;		//B
	int iTargetId;			//Y
	int iTargetUnitPrice;	//Z
};


