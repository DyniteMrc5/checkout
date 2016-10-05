//© 2016 Michael Cox
#pragma once

#include <string>
#include "deal.h"



/*
 * checkout - A shopping checkout implementation
 *
 */
namespace Checkout 
{
	constexpr int RECEIPT_WIDTH = 20;

	// Get permutations of deals
	std::vector<std::vector<const Deal*>> dealCombinations(std::vector<const Deal*> aDeals);

	// Filter out deals we do not need to process. 
	// (There are likely to be many more deals than items, and many of the deals will not be applicable to those items)
	std::vector<const Deal*> filterDeals(std::vector<const Deal*> aDeals, std::vector<Item>& aItems);

	std::string createReceipt(std::vector<std::tuple<const Deal*, Item, int>>& aInput, int aTotal);

	// prints receipt
	std::string checkoutItems(std::vector<Item>& aInput, std::vector<const Deal*>& aDeals, int& aTotal);
};


