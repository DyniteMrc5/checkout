//© 2016 Michael Cox
#include "checkout.h"
#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include <set>

#ifdef _MSC_VER
	// If editing in Visual Studio, define these
	// macros to help compilation and manual inspection.
	#define TEST(testclass, Title) void Title()
	#define ASSERT_EQ(X,Y)
	#define ASSERT_NE(X,Y)
#endif

// Print deal permutations
void print(std::vector<std::vector<const Deal*>> aInput)
{
	for (auto list : aInput)
	{
		std::cout << "	";
		for (auto* d : list)
		{
			std::cout << d << ",";
		}
		std::cout << std::endl;
	}
}

int factorial(int n)
{
	return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

void TestDealCombinations(size_t num)
{
	std::vector<const Deal*> deals{ num, nullptr };
	auto combs = Checkout::dealCombinations(deals);
	print(combs);
	ASSERT_EQ(combs.size(), factorial(num) + 1);
}

TEST(Basics, TestDealCombinations)
{
	TestDealCombinations(2);
}

TEST(Basics, TestDealCombinations2)
{
	TestDealCombinations(3);
}

TEST(Basics, TestDealCombinations3)
{
	TestDealCombinations(4);
}

TEST(Basics, Receipt)
{
	std::vector<const Deal*> deals{ };

	std::vector<Item> items{ Item(1, 100, std::string("Item1")) };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_NE(receipt.length(), 0);
}


TEST(Basics, TestInterviewCase)
{
	BuyAofXGetBofYForZ deal1(3, 1, 1, 1, 50); // Buy 3 of item1, get 1 item1 for 50
											//	should be (100 + 100 + (100 deal price as 50)) twice + 100
											//  == (100+100+50) + (100+100+50) + 100 == 600
	std::vector<const Deal*> deals{ &deal1 };

	std::vector<Item> items{ 7, Item(1, 100, std::string("Item1")) };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, 600);
}



TEST(Basics, TestNoValidDeal)
{
	BuyAofXGetBofYForZ deal1(3, 0, 1, 1, 0); // Buy 3 of item0, get 1 item1 for 0 
	std::vector<const Deal*> deals{ &deal1 };

	std::vector<Item> items{ Item(1, 100, std::string("Item1")) };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, 100);
}

void SimpleTest(int countSelector, size_t countOfItem, int expected)
{
	BuyAofXGetBofYForZ deal1(countSelector, 1, 1, 1, 50); // Buy 1 of item1, get 1 item1 for 50 
	std::vector<const Deal*> deals{ &deal1 };

	Item item1(1, 100, "Item1");
	std::vector<Item> items{ countOfItem, item1 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, expected);
}

TEST(Basics, TestWithSimpleDealSingleItemSingleDealValid)
{
	SimpleTest(1, 1, 50);
}

TEST(Basics, TestWithSimpleDealSingleItemSingleDealInValid)
{
	SimpleTest(2, 1, 100);
}

TEST(Basics, TestWithSimpleDealTwoItemsSingleDealValid)
{
	SimpleTest(2, 2, 150);
}

void TestWithOverlappingItems(size_t numItems, int expected)
{
	BuyAofXGetBofYForZ deal1(2, 1, 1, 1, 25); // Buy 2 of item1, get 1 item1 for 25 
	BuyAofXGetBofYForZ deal2(2, 1, 1, 1, 22); // Buy 2 of item1, get 1 item1 for 22 
	std::vector<const Deal*> deals{ &deal1, &deal2 };

	Item item1(1, 100, std::string("Item1"));
	std::vector<Item> items{ numItems, item1 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, expected);
}

TEST(Basics, TestWithOverlapDealTwoItemsDoubleDealBothValid)
{
	TestWithOverlappingItems(2, 122);
}

TEST(Basics, TestWithOverlapManySelectionItems)
{
	TestWithOverlappingItems(5, 344);
}

TEST(Basics, TestBadDeal)
{
	BuyAofXGetBofYForZ deal1(3, 1, 3, 1, 101); // Buy 3 of item1, get 3 item1 for 101 - this one should not be picked (101*3 > 100*3)
	std::vector<const Deal*> deals{ &deal1 };

	Item item1(1, 100, std::string("Item1"));
	std::vector<Item> items{ item1, item1, item1};
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, 300);
}

void TestCrossItemDeal(size_t numItem1s, size_t num2tem1s, int expected)
{
	BuyAofXGetBofYForZ deal1(3, 1, 1, 2, 22); // Buy 3 of item1, get 1 item2 for 22
	std::vector<const Deal*> deals{ &deal1 };

	Item item1(1, 100, "Item1");
	Item item2(2, 200, "Item2");

	std::vector<Item> items { numItem1s, item1 };
	for (int i = 0; i < num2tem1s; ++i)
	{
		items.push_back(item2);
	}

	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, expected);
}

TEST(Basics, TestCrossDeal_Valid)
{
	TestCrossItemDeal(3, 3, 722);
}

TEST(Basics, TestCrossDeal_InValid)
{
	TestCrossItemDeal(2, 3, 800);
}

TEST(Basics, TestCrossDeal_MultiManyValid)
{
	// 100, 100, 100, 22, 100, 100, 100, 22, 200
	TestCrossItemDeal(6, 3, 844);
}

TEST(Basics, TestCrossDeal_MultiManyValid_Reordered)
{
	BuyAofXGetBofYForZ deal2(1, 2, 1, 2, 33); // Buy 1 of item2, get 1 item2 for 33
	BuyAofXGetBofYForZ deal1(3, 1, 1, 2, 22); // Buy 3 of item1, get 1 item2 for 22
	std::vector<const Deal*> deals{ &deal1, &deal2 };

	Item item1(1, 100, std::string("Item1"));
	Item item2(2, 200, std::string("Item2"));
	std::vector<Item> items{ item1, item1, item1, item2, item2, item2,  item1, item1, item1 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, 677);
}

TEST(Basics, TestXInSetDeal_Basic)
{
	std::set<int> aInputSet{ 1 };
	BuyInSetOfXCheapestFree deal1{aInputSet, 2};
	std::vector<const Deal*> deals{ &deal1 };

	Item item1(1, 101, std::string("Item1"));
	std::vector<Item> items{ item1, item1, item1 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, 202);
}

TEST(Basics, TestXInSetDeal_Basic_DifferentItems)
{
	std::set<int> aInputSet{ 1, 2 };
	BuyInSetOfXCheapestFree deal1{ aInputSet, 2 };
	std::vector<const Deal*> deals{ &deal1 };

	Item item1(1, 101, std::string("Item1"));
	Item item2(2, 201, std::string("Item2"));
	std::vector<Item> items{ item1, item2 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, 201);
}


TEST(Basics, TestXInSetDeal_Basic_MultipleItems)
{
	std::set<int> aInputSet{ 1, 2 };
	BuyInSetOfXCheapestFree deal1{ aInputSet, 2 };
	std::vector<const Deal*> deals{ &deal1 };

	Item item1(1, 101, std::string("Item1"));
	Item item2(2, 201, std::string("Item1"));
	std::vector<Item> items{ item1, item2, item2 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, 402);
}

TEST(Basics, TestDeserialise_BuyInSetOfXCheapestFree)
{
	std::set<int> aInputSet{ 1, 2 };
	BuyInSetOfXCheapestFree deal1{ aInputSet, 2 };

	std::string expected = std::to_string(EBuyInSetOfXCheapestFree) + " 2 1 2";
	std::string s = deal1.serialise();

	ASSERT_EQ(s, expected);
}

TEST(Basics, TestSerialise_BuyInSetOfXCheapestFree)
{
	std::set<int> input{ 1, 2 };

	std::string str = std::to_string(EBuyInSetOfXCheapestFree) + " 2 1 2";
	std::shared_ptr<Deal> shared_deal = Deal::deserialise(str);

	Deal* deal = shared_deal.get();
	BuyInSetOfXCheapestFree* d = static_cast<BuyInSetOfXCheapestFree*>(deal);

	ASSERT_EQ(d->selection() , input);
	ASSERT_EQ(d->targetCount() , 2);
}


TEST(Basics, TestDeserialise_BuyAofXGetBofYFZ)
{
	BuyAofXGetBofYForZ deal1{ 1, 2, 3, 4, 5 };

	std::string expected = std::to_string(EBuyAofXGetBofYFZ) + " 1 2 3 4 5";
	std::string s = deal1.serialise();

	ASSERT_EQ(s, expected);
}

TEST(Basics, TestSerialise_BuyAofXGetBofYFZ)
{
	std::string str = std::to_string(EBuyAofXGetBofYFZ) + " 1 2 3 4 5";
	std::shared_ptr<Deal> shared_deal = Deal::deserialise(str);

	Deal* deal = shared_deal.get();
	BuyAofXGetBofYForZ* d = static_cast<BuyAofXGetBofYForZ*>(deal);

	ASSERT_EQ(d->selectionCount(), 1);
	ASSERT_EQ(d->selectionId(), 2);
	ASSERT_EQ(d->targetCount(), 3);
	ASSERT_EQ(d->targetId(), 4);
	ASSERT_EQ(d->targetUnitPrice(), 5);
}


TEST(Basics, TestSmartDeals_SingleItem)
{
	Item item1{ 1, 100, "Crisps" };
	SingleItemSelector singleItemSelector{ item1 };

	DealSelectorSelectTargetPrice dsSTP{ std::make_tuple(&singleItemSelector, &singleItemSelector, 80) };
	StrictDealSelector deal(dsSTP);
	std::vector<DealSelector*> selectors{ &deal };
	MultiDealSelector ds(selectors);
	SmartDeal deal1(ds);

	std::vector<const Deal*> deals{ &deal1 };

	std::vector<Item> items{ item1 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, 80);
}


int TestDeals_Generic(int countSelections, int countTargets, std::vector<Item>::size_type countNumItems)
{
	Item item1{ 1, 100, "Item B" + std::to_string(countSelections) + "G" + std::to_string(countTargets) + "#" + std::to_string(countNumItems) };

	BuyAofXGetBofYForZ deal1(countSelections, 1, countTargets, 1, 50);
	deal1.name() = "DEAL BuyXGetX";

	std::vector<const Deal*> deals{ &deal1 };

	std::vector<Item> items{ countNumItems, item1 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	return total;
}

//For each N (equal) items of X you get K items of Y for Z
TEST(Basics, TestDeals_InterviewCase_Generic)
{
	for (int numTargets = 1; numTargets <= 3; ++numTargets)
	{
		for (int numSelectors = numTargets; numSelectors <= 3; ++numSelectors)
		{
			for (std::vector<Item>::size_type numItems = 1; numItems <= 9; ++numItems)
			{
				int expected = numItems * 100;
				expected -= (numItems / numSelectors) * numTargets * 50;

				std::cout << "TestDeals_InterviewCase_Generic:" <<
					"Buy " << numSelectors <<
					", Get " << numTargets <<
					", #" << numItems << " Items" <<
					" (Expected: " << expected << ")" << std::endl;
				ASSERT_EQ(TestDeals_Generic(numSelectors, numTargets, numItems), expected);
			}
		}
	}
}


int TestSmartDeals_Generic(int countSelections, int countTargets, std::vector<Item>::size_type countNumItems)
{
	Item item1{ 1, 100, "Item B" + std::to_string(countSelections) + "G" + std::to_string(countTargets) + "#" + std::to_string(countNumItems) };
	CountedSpecificItemSelector selectionSelector{ item1, countSelections }; // # of sweets to qualify
	CountedSpecificItemSelector targetSelector{ item1, countTargets }; // # of sweets affected

	DealSelectorSelectTargetPrice dsSTP{ std::make_tuple(&selectionSelector, &targetSelector, 50) };
	StrictDealSelector deal(dsSTP);
	std::vector<DealSelector*> selectors{ &deal };
	MultiDealSelector ds(selectors);
	SmartDeal deal1(ds);
	deal1.name() = "DEAL BuyXGetX";

	std::vector<const Deal*> deals{ &deal1 };

	std::vector<Item> items{ countNumItems, item1 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	return total;
}

//For each N (equal) items of X you get K items of Y for Z
TEST(Basics, TestSmartDeals_InterviewCase_Generic)
{
	ASSERT_EQ(TestSmartDeals_Generic(3, 1, 7), 600);

	for (int numTargets = 1; numTargets <= 3; ++numTargets)
	{
		for (int numSelectors = numTargets; numSelectors <= 3; ++numSelectors)
		{
			for (std::vector<Item>::size_type numItems = 1; numItems <= 9; ++numItems)
			{
				int expected = numItems * 100;
				expected -= (numItems / numSelectors) * numTargets * 50;

				std::cout << "TestSmartDeals_InterviewCase_Generic:" << 
					    "Buy " 	<< numSelectors << 
						", Get " << numTargets << 
						", #" << numItems << " Items" <<
						" (Expected: " << expected << ")" << std::endl;
				ASSERT_EQ(TestSmartDeals_Generic(numSelectors, numTargets, numItems), expected);
			}
		}
	}
	
}

//For each N (equal) items of X you get K items of Y for Z
TEST(Basics, TestSmartDeals_InterviewCase)
{
	Item item1{ 1, 100, "Sweets" };
	CountedSpecificItemSelector selectionSelector{ item1, 3 }; // 3 lots of sweets
	CountedSpecificItemSelector targetSelector{ item1, 1 }; // 1 lots of sweets

	DealSelectorSelectTargetPrice dsSTP{ std::make_tuple(&selectionSelector, &targetSelector, 50) };
	StrictDealSelector deal(dsSTP);
	std::vector<DealSelector*> selectors{ &deal };
	MultiDealSelector ds(selectors);
	SmartDeal deal1(ds);
	deal1.name() = "Interview Case - Smart";

	std::vector<const Deal*> deals{ &deal1 };

	std::vector<Item> items{ 7, item1 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, 600);

}

//For each N (equal) items of X you get K items of Y for Z
TEST(Basics, TestSmartDeals_ForNofX_GetKofYforZ)
{
	//StrictDealSelector[0] = <CountedSpecificItemSelector(N)[A], CountedSpecificItemSelector(K)[Y], UnitPrice[Z]>
	Item item1{ 1, 99, "Sweets" };
	Item item2{ 1, 100, "Cake" };
	CountedSpecificItemSelector sweetsSelector{ item1, 3 }; // 3 lots of sweets
	CountedSpecificItemSelector cakeSelector{ item2, 1 }; // 1 lots of cake

	DealSelectorSelectTargetPrice dsSTP{ std::make_tuple(&sweetsSelector, &cakeSelector, 50) };
	StrictDealSelector deal(dsSTP);
	std::vector<DealSelector*> selectors{ &deal };
	MultiDealSelector ds(selectors);
	SmartDeal deal1(ds);
	deal1.name() = "DealSweets&Cakes";

	std::vector<const Deal*> deals{ &deal1 };

	std::vector<Item> items{ item1, item1, item1, item2, item2 };
	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, 447);

}


void TestSmartDeals_TescoMealDeal_ExtraItems(bool optionalSelector, bool optionalPizza)
{
	Item sandwich1{ 1, 175, "Sandwich1" };
	Item sandwich2{ 2, 145, "Sandwich2" };
	Item sandwich3{ 3, 155, "Sandwich3" };

	std::set<Item> sandwiches{ sandwich1, sandwich2, sandwich3 };

	Item drink1{ 4, 80, "Drink1" };
	Item drink2{ 5, 85, "Drink2" };
	Item drink3{ 6, 86, "Drink3" };

	std::set<Item> drinks{ drink1, drink2, drink3 };

	Item crisps1{ 7, 110, "Crisps1" };
	Item crisps2{ 8, 101, "Crisps2" };
	Item crisps3{ 9, 105, "Crisps3" };

	Item pizza{ 10, 500, "Pizza " };

	std::set<Item> crisps{ crisps1, crisps2, crisps3 };

	// Define Selection lookups:
	SingleInSetSelector sandwichesSelector{ sandwiches };
	SingleInSetSelector crispsSelector{ crisps };
	SingleInSetSelector drinkSelector{ drinks };

	SingleItemSelector pizzaSelector{ pizza };

	// Define Selection and Target pairs, with price
	DealSelectorSelectTargetPrice sandwichSTP{ std::make_tuple(&sandwichesSelector, &sandwichesSelector, 100) };
	DealSelectorSelectTargetPrice cripsSTP{ std::make_tuple(&crispsSelector, &crispsSelector, 100) };
	DealSelectorSelectTargetPrice drinkSTP{ std::make_tuple(&drinkSelector, &drinkSelector, 100) };
	DealSelectorSelectTargetPrice pizzaSTP{ std::make_tuple(&pizzaSelector, &pizzaSelector, 350) };

	// Are these strict or optional ?
	StrictDealSelector sandwichDealSelector(sandwichSTP);
	StrictDealSelector cripsDealSelector(cripsSTP);
	StrictDealSelector drinkDealSelector(drinkSTP);
	OptionalDealSelector pizzaDealSelector(pizzaSTP);

	std::vector<DealSelector*> selectors{ &sandwichDealSelector, &cripsDealSelector, &drinkDealSelector};

	if (optionalSelector)
	{
		selectors.push_back(&pizzaDealSelector);
	}

	MultiDealSelector ds(selectors);
	SmartDeal deal1(ds);
	deal1.name() = "Tesco Meal Deal";

	std::vector<const Deal*> deals{ &deal1 };

	std::vector<Item> items{ sandwich1, drink2, crisps3, drink1 };

	int expected = 385;

	if (optionalPizza)
	{
		items.push_back(pizza);
		
		if (optionalSelector)
		{
			expected += 350;
		}
		else {
			expected += 500;
		}
	} 

	int total;
	std::string receipt = Checkout::checkoutItems(items, deals, total);
	std::cout << receipt << std::endl;
	ASSERT_EQ(total, expected);
}


TEST(Basics, TestSmartDeals_TescoMealDeal_ExtraItems_StrictOnly)
{
	TestSmartDeals_TescoMealDeal_ExtraItems(false, false);
}

TEST(Basics, TestSmartDeals_TescoMealDeal_ExtraItems_StrictOnly_WithPizza)
{
	TestSmartDeals_TescoMealDeal_ExtraItems(false, true);
}

TEST(Basics, TestSmartDeals_TescoMealDeal_ExtraItems_WithOptionalPizzaDeal_WithoutPizza)
{
	TestSmartDeals_TescoMealDeal_ExtraItems(true, false);
}

TEST(Basics, TestSmartDeals_TescoMealDeal_ExtraItems_WithOptionalPizzaDeal_WitPizza)
{
	TestSmartDeals_TescoMealDeal_ExtraItems(true, true);
}




