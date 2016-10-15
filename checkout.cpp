#include "checkout.h"
#include <map>
#include <algorithm>
#include <iostream>
#include <tuple>
#include <limits>

/*
 * Given our original input and our checkout output
 * Calculate the total bill price.
 */
int calculateTotalPrice(std::vector<Item>& aInput, std::vector<std::pair<Item, int>>& aDealItems)
{
	int total = 0;

	// For each item
	for (Item& item : aInput)
	{
		// Check whether to add a deal price for item
		bool addedInDeal = false;
		for (std::pair<Item, int>& pair : aDealItems)
		{
			if (pair.first == item)
			{ 
				total += pair.second;
				addedInDeal = true;
				break;
			}
		}

		// Add original price
		if (!addedInDeal)
		{
			total += item.iUnitPrice;
		}
	}
	return total;
}

/*
 * Create all permutations of Deals.
 * This is because the order we evaluate Deals is significant.
 */
template <typename T>
std::vector<std::vector<const T*>> permuations(std::vector<const T*> aDeals)
{
	std::vector<std::vector<const T*>> result;

	if (aDeals.size() == 0)
	{
		// return [[]]
		result.push_back(std::vector<const T*>{ });
		return result;
	}

	if (aDeals.size() == 1)
	{
		// return [[deal1]]
		result.push_back(std::vector<const T*>{aDeals[0]});
		return result;
	}

	const T* d = aDeals[0];
	std::vector<const T*> sublist(aDeals.begin() + 1, aDeals.end());

	std::vector<std::vector<const T*>> perms = permuations(sublist);

	int numPerms = perms.size();
	for (int p = 0; p < numPerms; ++p)
	{
		auto& permutation = perms[p];
		// insert d at every index;
		for (int i = 0; i <= permutation.size(); ++i)
		{
			std::vector<const T*> permutation_copy = permutation;
			permutation_copy.insert(permutation_copy.begin() + i, d);
			result.push_back(permutation_copy);
		}
	}
	return result;

}

/*
* Create all permutations of Deals.
* This is because the order we evaluate Deals is significant.
* Empty list is added for the case where we have no Deals.
*/
std::vector<std::vector<const Deal*>> Checkout::dealCombinations(std::vector<const Deal*> aDeals)
{
	auto perms = ::permuations<Deal>(aDeals);
	perms.push_back({}); //add empty list - default case
	return perms;
}

/* 
 * We will probably have < 300 items, but may have many thousands of deals, so
 * use this function to filter out the deals that are not valid.
 * Will hopefully be a set lookup, or object comparison, which should be performant.
 */
std::vector<const Deal*> Checkout::filterDeals(std::vector<const Deal*> aDeals, std::vector<Item>& aItems)
{
	std::vector<const Deal*> result;

	for (const Deal* d : aDeals)
	{
		for (Item& item : aItems)
		{
			if (d->selectsOn(item) || d->targets(item))
			{
				result.push_back(d);
				break;
			}
		}
	}

	return result;
}

/*
 * Once the items have been processed (with Checkout::checkoutItems) and
 * the best deal permutation has been identified,
 * this function can build a printable receipt.
 */
std::string Checkout::createReceipt(std::vector<std::tuple<const Deal*, Item, int>>& aInput, int aTotal)
{
	std::string receiptStr = "RECEIPT";
	std::string headingFill = std::string((RECEIPT_WIDTH - receiptStr.length()) / 2, ' ');
	std::string receipt = headingFill + receiptStr + headingFill + "\n";

	receipt += std::string(RECEIPT_WIDTH, '-') + '\n';

	// For every item (which may/may not have an associated Deal)...
	for (std::tuple<const Deal*, Item, int>& tuple : aInput)
	{
		const Deal* deal = std::get<0>(tuple);
		int id = std::get<1>(tuple).iId;
		int original_price = std::get<1>(tuple).iUnitPrice;
		int price = std::get<2>(tuple);
		
		std::string idStr = std::get<1>(tuple).name();
		std::string originalPriceStr = std::to_string(original_price); 

		// If we have a deal fo this item, we
		// put the original price in brackets,
		// and the deal price will be on the next line
		if (original_price != price)
		{
			originalPriceStr = "(" + originalPriceStr + ")";
		}

		std::string priceStr = std::to_string(price);

		// Print ITEM
		std::string line(RECEIPT_WIDTH, ' ');
		line.replace(line.begin(), line.begin() + idStr.length(), idStr);

		// Print PRICE
		std::string receipt_price = (deal) ? originalPriceStr : priceStr;
		line.replace(line.begin() + RECEIPT_WIDTH - receipt_price.length(), line.end(), receipt_price);
		line += '\n';
    	receipt += line;

		// Insert Deal Info:
		if (deal && original_price != price) // if this price was affected by deal
		{
			std::string line(RECEIPT_WIDTH, ' ');
			std::string name = deal->name();

			//don't print all of name if it doesn't fit
			int nameLen = std::min((int)name.length(), RECEIPT_WIDTH); 
			auto endOfDealNameIter = line.begin() + nameLen;

			line.replace(line.begin(), endOfDealNameIter, name);

			// Do we have space to write the name and price?
			int endOfDealNameIdx = nameLen + priceStr.length() + 1;
			auto startOfPriceIdx = RECEIPT_WIDTH - priceStr.length();

			// Add elipsis to show text has been cut off..
			if (endOfDealNameIdx > startOfPriceIdx)
			{
				auto elipsisIter = line.begin() + RECEIPT_WIDTH - priceStr.length() - 4;
				line.replace(elipsisIter, elipsisIter + 4, "... ");
			}

			line.replace(line.begin() + startOfPriceIdx, line.end(), priceStr);
			line += '\n';
			receipt += line;
		}
	}

	receipt += std::string(RECEIPT_WIDTH, '-');
	receipt += '\n';

	std::string line(RECEIPT_WIDTH, ' ');
	line.replace(line.begin(), line.begin() + 6, "Total:");
	std::string totalStr = std::to_string(aTotal);
	auto totalIter = line.begin() + line.length() - totalStr.length();
	line.replace(totalIter, totalIter + totalStr.length(), totalStr);
	line += '\n';
	receipt += line;

	return receipt;
}

/*
 Check out list of Items

 Approach:
  We have a number of deals.
  We need to find the optimal receipt (for the customer)
  This means that we need to find the best deal covering the input range.
  The problem is that a deals may overlap, meaning that all combinations ("permutations") of deals need to be evaluated
  
 Method: 
   Get all permutation of deals.
   For each permutation:
     Evaluate each deal in turn, providing the remaining input. 
     For each evaluation, remove the affected input from the list
     Continue to evaluate a deal until no more results are returned. Then proceed to next deal.
     Save the permutation if its the best

  Returns the checkout receipt
 */
std::string Checkout::checkoutItems(std::vector<Item>& aInput, std::vector<const Deal*>& aDeals, int& aTotal)
{

	// (performance optimisation) Remove deals which do not affect aInput
	// - likely to only be a few relevant deals for our Items
	aDeals = filterDeals(aDeals, aInput);

	std::map<int, int> bestDeal;
	int lowest_total = std::numeric_limits<int>::max();
	std::vector<std::tuple<const Deal*, Item, int>> best_result{};

	// Get deal permutations
	std::vector<std::vector<const Deal*>> dealPermutations = Checkout::dealCombinations(aDeals);

	//Iterate permutations
	for (int i = 0; i < dealPermutations.size(); ++i)
	{
		std::vector<const Deal*>& dealPermutation = dealPermutations[i];
		
		//copy input (we need to modify it if we get a match - items are not applicable to multiple deals)
		std::vector<Item> input = aInput;

		std::vector<std::tuple<const Deal*, Item, int>> current_result{};

		// For each deal in this permutation:
		//   evaluate the deal on the input.
		//   We have to do this repetitively for a specific deal (value of j) until the deal find no more matching selections in the input

		for (int j = 0; j < dealPermutation.size(); /* increment when evaluate returns no result */)
		{
			const Deal* deal = dealPermutation[j];

			// Evaluate - storing any items affected and the resultant unit price
			std::vector<std::pair<Item, int>> result = deal->evaluate(input);

			int numAffectItems = result.size();
			if (numAffectItems == 0)
			{
				// No items matched. Go to next deal.
				++j;
				continue;
			}

			// Deal matched some items...

			// Remove items from input (deals cannot be used in conjunction)
			std::vector<std::pair<Item, int>>::const_iterator it = result.begin();
			while (it != result.end())
			{
				std::tuple<const Deal*, Item, int> tuple = std::make_tuple(deal, it->first, it->second);
				current_result.push_back(tuple);

				const Item& item = (*it).first;
				auto find = std::find(input.begin(), input.end(), item);

				// May have already been removed (e.g. smart deals)
				if (find != input.end())
				{
					input.erase(find);
				}
				it++;
			}

			// Add deal results to map
			int currentValue = bestDeal[i];

			for (std::pair<Item, int>& pair : result)
			{
				currentValue += pair.second;
			}
			bestDeal[i] = currentValue;
		}
		
		// Add any values which have not been matched by a deal
		for (Item item : input)
		{
			std::tuple<Deal*, Item, int> p = std::make_tuple(nullptr, item, item.iUnitPrice);
			current_result.push_back(p);
			bestDeal[i] += item.iUnitPrice;
		}

		if (bestDeal[i] < lowest_total)
		{
			lowest_total = bestDeal[i];
			best_result = current_result;
		}
	}
	
	aTotal = lowest_total;

	// Generate receipt
	return createReceipt(best_result, lowest_total);
}
