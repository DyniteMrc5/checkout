#include "checkout.h"
#include <map>
#include <algorithm>
#include <iostream>
#include <tuple>
#include <limits>

int calculateTotalPrice(std::vector<Item>& aInput, std::vector<std::pair<Item, int>>& aDealItems)
{
	int total = 0;

	for (Item& item : aInput)
	{
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

		if (!addedInDeal)
		{
			total += item.iUnitPrice;
		}
	}
	return total;
}


std::vector<std::vector<const Deal*>> permuations(std::vector<const Deal*> aDeals)
{
	std::vector<std::vector<const Deal*>> result;

	if (aDeals.size() == 0)
	{
		// return [[]]
		result.push_back(std::vector<const Deal*>{ });
		return result;
	}

	if (aDeals.size() == 1)
	{
		// return [[deal1]]
		result.push_back(std::vector<const Deal*>{aDeals[0]});
		return result;
	}

	const Deal* d = aDeals[0];
	std::vector<const Deal*> sublist(aDeals.begin() + 1, aDeals.end());

	std::vector<std::vector<const Deal*>> perms = permuations(sublist);

	int numPerms = perms.size();
	for (int p = 0; p < numPerms; ++p)
	{
		auto& permutation = perms[p];
		// insert d at every index;
		for (int i = 0; i <= permutation.size(); ++i)
		{
			std::vector<const Deal*> permutation_copy = permutation;
			permutation_copy.insert(permutation_copy.begin() + i, d);
			result.push_back(permutation_copy);
		}
	}
	return result;

}

std::vector<std::vector<const Deal*>> Checkout::dealCombinations(std::vector<const Deal*> aDeals)
{
	auto perms = permuations(aDeals);
	perms.push_back({}); //add empty list - default case
	return perms;
}

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


	for (std::tuple<const Deal*, Item, int>& tuple : aInput)
	{
		const Deal* deal = std::get<0>(tuple);
		int id = std::get<1>(tuple).iId;
		int original_price = std::get<1>(tuple).iUnitPrice;
		int price = std::get<2>(tuple);
		
		std::string idStr = std::get<1>(tuple).name();
		std::string originalPriceStr = std::to_string(original_price); 

		if (original_price != price)
		{
			originalPriceStr = "(" + originalPriceStr + ")";
		}

		std::string priceStr = std::to_string(price);

		std::string line(RECEIPT_WIDTH, ' ');
		line.replace(line.begin(), line.begin() + idStr.length(), idStr);

		std::string receipt_price = (deal) ? originalPriceStr : priceStr;

		line.replace(line.begin() + RECEIPT_WIDTH - receipt_price.length(), line.end(), receipt_price);
		line += '\n';
    	receipt += line;

		// Insert Deal Info:
		if (deal && original_price != price) // if this price was affected by deal
		{
			std::string line(RECEIPT_WIDTH, ' ');
			std::string name = deal->name();

			int nameLen = std::min((int)name.length(), RECEIPT_WIDTH); //don't print too name if it doesn't fit
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


std::string Checkout::checkoutItems(std::vector<Item>& aInput, std::vector<const Deal*>& aDeals, int& aTotal)
{

	// Approach:
	//	We have a number of deals.
	//	We need to find the optimal receipt (for the customer)
	//  This means that we need to find the best deal covering the input range.
	//  The problem is that a deals may overlap, meaning that all combinations ("permutations") of deals need to be evaluated
	//  
	// Method: 
	//   Get all permutation of deals.
	//   For each permutation:
	//	   Evaluate the each deal in turn, providing the input. 
	//     For each evaluation, remove the affected input from the list
	//	   Continue to evaluate a deal until no more results are returned. Then proceed to next deal.
	//	   Save the total for that run in a hashmap (index in to dealPermutations)


	// (performance optimisation) Remove deals which do not affect aInput - likely to only be a few relevant deals for our Items
	aDeals = filterDeals(aDeals, aInput);

	std::map<int, int> bestDeal;
	int lowest_total = std::numeric_limits<int>::max();
	std::vector<std::tuple<const Deal*, Item, int>> best_result{};

	std::vector<std::vector<const Deal*>> dealPermutations = Checkout::dealCombinations(aDeals);
	for (int i = 0; i < dealPermutations.size(); ++i)
	{
		std::vector<const Deal*>& dealPermutation = dealPermutations[i];
		
		//copy input
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

			// Deal matched some items

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

	//std::cout << "Lowest Total:" << lowest_total << std::endl;
	
	aTotal = lowest_total;
	return createReceipt(best_result, lowest_total);
}
