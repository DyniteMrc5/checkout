#pragma once

#include <set>
#include <vector>
#include <tuple>
#include <memory>

#include "item.hpp"
#include "deal.h"

// Abstract Selector
class Selector
{
public:
	virtual std::vector<Item> select(std::vector<Item>& aItems) = 0;
};

// --------------

// Looks for a single specific item
class SingleItemSelector : public Selector
{
public:
	SingleItemSelector(Item& aItem) : Selector(), iSelectionItem(aItem) {};

	virtual std::vector<Item> select(std::vector<Item>& aItems);
protected:
	Item& iSelectionItem;
};


/*
* Matches a specified number of specific item
*/
class CountedSpecificItemSelector : public SingleItemSelector
{
public:
	CountedSpecificItemSelector(Item& aItem, int aSelectionCount)
		: SingleItemSelector(aItem), iSelectionCount(aSelectionCount)
	{
	};

	virtual std::vector<Item> select(std::vector<Item>& aItems);
private:
	int iSelectionCount;
};


// Abstract Selector
class ManyItemSelector : public Selector
{
public:
	virtual std::vector<Item> select(std::vector<Item>& aItems) = 0;
protected:
	ManyItemSelector(std::set<Item>& aSelection) : iSelectionSet(aSelection) {};

	std::set<Item>& iSelectionSet;
};

class GreedyAnyInSetSelector : public ManyItemSelector
{
public:
	GreedyAnyInSetSelector(std::set<Item>& aSelection) :
		ManyItemSelector(aSelection)
	{};

	virtual std::vector<Item> select(std::vector<Item>& aItems);
};

/*
* Matches if an Item is in the set
*/
class CountedAnyInSetSelector : public ManyItemSelector
{
public:
	CountedAnyInSetSelector(std::set<Item>& aSelection, int aCount) :
		ManyItemSelector(aSelection), iSelectionCount(aCount)
	{};

	int iSelectionCount;
	virtual std::vector<Item> select(std::vector<Item>& aItems);
};

/*
* Matches if an Item is in the set
*/
class CountedCheapestInSetSelector : public CountedAnyInSetSelector
{
public:
	CountedCheapestInSetSelector(std::set<Item>& aSelection, int aCount)
		: CountedAnyInSetSelector(aSelection, aCount) 
	{};

	virtual std::vector<Item> select(std::vector<Item>& aItems);
};

/*
* Matches if an Item is in the set
*/
class SingleInSetSelector : public CountedCheapestInSetSelector
{
public:
	SingleInSetSelector(std::set<Item>& aSelection) :
		CountedCheapestInSetSelector(aSelection, 1)
	{};

	virtual std::vector<Item> select(std::vector<Item>& aItems);
};



// ---- Combining Selectors:

typedef Selector SelectionSelector;
typedef Selector TargetSelector;

/* MultiDealSelector has tuples of <Selectors, Targets, and a target unit price>
   This allows to create a mix 'n' match of selections, each which could have its own unit price.
   Burden is on user to provide unit price for all aggregated targets
   
   Some examples:
   
   To Create: Buy 1, Get 1 Free
	StrictDealSelector[0] = <CountedCheapestInSetSelector(2)[A, B, or C], SingleInSetSelector[A, B, C], UnitPrice[0]>

   To Create: Buy 3 (equal) items, Pay for 2
	StrictDealSelector[0] = <CountedSpecificItemSelector(3)[A], CountedSpecificItemSelector(1)[A], UnitPrice[0]>

   To Create: Buy 2 (equal) items for a special price
	StrictDealSelector[0] = <CountedSpecificItemSelector(2)[A], CountedSpecificItemSelector(2)[A], UnitPrice[special_price / 2]>
   Note: the price is averaged across the number of items

   To Create: Buy 3 (in a set of items) and the cheapest is free:
	StrictDealSelector[0] = <CountedCheapestInSetSelector(3)[A, B, C, D, E], CountedCheapestInSetSelector(1)[A, B, C, D, E], UnitPrice[0]>
 
   To Create: For each N (equal) items of X you get K items of Y for free
	StrictDealSelector[0] = <CountedSpecificItemSelector(N)[A], CountedSpecificItemSelector(K)[Y], UnitPrice[0]>

   To Create: If you buy any (in a set of items), you get some other item X for P
   e.g. if you buy anything (any number) from this bargin bin, take a box of close-to-expired chocolates
	StrictDealSelector[0] = <GreedyAnyInSetSelector[A, B, C, D, ...], SingleInSetSelector[X,..], UnitPrice[P]>

   To create: a Meal Deal for 300 (
	StrictDealSelector[0] = <SingleInSetSelector[A, B, or C], SingleInSetSelector[A, B, or C], UnitPrice[100]>   // sandwich
	StrictDealSelector[1] = <SingleInSetSelector[X or Y],	   SingleInSetSelector[X,  Y],	    UnitPrice[100]>   // crisps
	StrictDealSelector[2] = <SingleInSetSelector[Q or W],	   SingleInSetSelector[Q, W],	    UnitPrice[100]>   // drink
   NB: Price is spread evently across included products

   OptionalDealSelector allows optional parts of a deal.
   E.g. To Create a Meal deal - 3 courses = 300, 4 courses = 400 (an optional 4th course):
	StrictDealSelector[0] =   <SingleInSetSelector[A, B, or C], SingleInSetSelector[A, B, or C], UnitPrice[100]>   // sandwich
	StrictDealSelector[1] =   <SingleInSetSelector[X or Y],	 SingleInSetSelector[X,  Y],	  UnitPrice[100]>   // crisps
	StrictDealSelector[2] =   <SingleInSetSelector[Q or W],	 SingleInSetSelector[Q, W],	      UnitPrice[100]>   // drink
	OptionalDealSelector[3] = <SingleInSetSelector[M or N],	 SingleInSetSelector[M or N],	  UnitPrice[100]>   // optional desert

 */

typedef std::tuple<SelectionSelector*, TargetSelector*, int> DealSelectorSelectTargetPrice;

/*
 * Represents a "simple" deal, i.e. a selection selector, a target selector and a price
 */
class DealSelector
{
protected:
	// NB Protected to prevent direct creation
	DealSelector(DealSelectorSelectTargetPrice& aSelector) : iSelector(aSelector) {};

public:
	DealSelectorSelectTargetPrice& iSelector;

	virtual bool strict() = 0;
};

class StrictDealSelector : public DealSelector
{
public:
	StrictDealSelector(DealSelectorSelectTargetPrice& aSelector) : DealSelector(aSelector) {};

	virtual bool strict() {
		return true;
	};
};

class OptionalDealSelector : public DealSelector
{
public:
	OptionalDealSelector(DealSelectorSelectTargetPrice& aSelector) : DealSelector(aSelector) {};

	virtual bool strict() {
		return false;
	};
};

/*
* Represents multiple DealSelectors
*/
class MultiDealSelector
{
public:
	// NB: Protected to prevent direct construction
	MultiDealSelector(std::vector<DealSelector*>& aSelectors)
		: iSelectors(aSelectors)
	{};

	std::vector<DealSelector*>& selectors() { return iSelectors; };

private:
	std::vector<DealSelector*>& iSelectors;

};

