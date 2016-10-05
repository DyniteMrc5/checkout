# checkout
A shopping checkout implementation in c++

## Clone, Build and Run
  >git clone --recursive https://github.com/DyniteMrc5/checkout
  
(--recursive needed to also download the googletest submodule)

  >make
  
  >./checkout_test

This will run the test harness which runs a number of tests, some of which output a (fixed width) receipt such as the below.

    [ RUN      ] Basics.TestSmartDeals_TescoMealDeal_ExtraItems
          RECEIPT
    --------------------
    Sandwich1      (175)
    Tesco Meal De... 100
    Crisps3        (105)
    Tesco Meal De... 100
    Drink1          (80)
    Tesco Meal De... 100
    Drink2            85
    Pizza            500
    --------------------
    Total:           885

    [       OK ] Basics.TestSmartDeals_TescoMealDeal_ExtraItems (0 ms)

## Architecture

The architecure resolves around the deals.
Deals can be arbitrarily complex, combined and restricted.

The deal architecure itself is object oriented in order to allow future expansion, however the basic structure included involves a number of powerful building blocks.

### (Shopping) Items

Items are simple objects in this system. They have an integer id, unit price and they have a string name.

### Deals (Selectors, Targets and Unit Price)

Strictly, a Deal is defined as a subclass of the `Deal` abstract class.
The `Deal` class defines a few pure virtual methods which must be implemeted in subclasses.

Deals evaluate a given input (the shopping items) and if the deal applies, the relevant items are returned with a new unit price.

The simplest Deal is perhaps 'Buy 1 Get 1 Free'. This can be generalised to 'Buy #A of ItemB, Get #C of ItemD for P Unit Price' (ItemB and ItemD could be the same Item)

The following can all be modelled with the generalisation above: 
  - buy 3 (equal) items and pay for 2
  - buy 2 (equal) items for a special price
  - for each N (equal) items of X you get K items of Y for free
  
However, the following don't fit this model:

  - buy 3 (in a set of items) and the cheapest is free
  - buy 1 (in a set of items) AND buy 1 (in a set of items) AND (buy 1 in a set of items) -> For a fixed price. ("Tesco meal deal")

In order to model the 'Meal Deal' concept, it is necessary to be able to have more expressive power and be able to combine 'sub-deals'.

This is done with `Selector`s.

A simple deal (BOGOF) is defined as a single `DealSelector` which is a tuple containing 
`SelectionSelectors`, `TargetSelectors` and a unit price (int).

#### Selectors

By example,

In a typical meal deal you choose a sandwich, crisps and drink for a predefined selection.

It is possible to mimic this by having a 'sandwich selector', 'drink selector' and 'crisps selector'.

Lets say, our shopping trolley contains: Sandwich1, Sandwich2, Drink1 and Crisps1.
We provide these inputs to our sandwich selector, and would matches against Sandwich1 and Sandwich2, both are eligable for the deal, but it only 'selects' 1. (It will first choose the cheaper of the two items).

The same is done for the drinks and crisps selector.

The result is that we have:
  Sandwich1 -> Chosen by SandwichSelector,
  Drink1 -> Chosen by DrinkSelector,
  Crisps1 -> Chosen by CrispSelector

These items are removed from the input. Then it is important that we look again, as we may be buying 2 meals.

On the second iteration, the sandwich selector will select Sandwich2. However, we do not have qualifying Drink or Crisps.
To handle this, the Selectors can be either strict or optional. Strict means that all DealSelectors (sandwich, crisps, drink), must find qualifying items.

(We could also include an Optional 'Desert' DealSelector)

#### SelectionSectors and TargetSelectors

These are actually the exact same class (`Selector`), but used in a slightly different way.
The SelectionSectors define which items to look for. The TargetSelectors define which items to look for *to apply the new price to*.

In the meal example, or buy 1 get 1 free example, it is the same items we are dealing with.
However, in the case of Buy 2 of X, get 1 of Y for Z price, we are selecting Xs, but we are targetting our new price against Y.
To handle this, we always define the SelectionSelectors and TargetSelectors.

#### UnitPrice

The UnitPrice is defined at the Deal level. This means that if you are defining a meal deal for 300 units and there are 3 sub-deals (or we are using a `MultiDealSelector` with 3 DealSelectors (where a `DealSelector` is a tuple of `<SelectionSelector, TargetSelectors, (int) UnitPrice>`), you have to give each part 100 units (or 1 300, and the other 2 0 - however you want).

#### Types of Selectors

In order to build a complex deal, we need to be able to express complex selections.
There are the following types of Selectors:

  // Looking for Single item:
  SingleItemSelector // Selects 1 of `Item`
  CountedSpecificItemSelector : public SingleItemSelector // Selects #X of `Item`

  // Looking in a set of items:
  ManyItemSelector : public Selector // Abstract
  CountedAnyInSetSelector : public ManyItemSelector //Selects X `Items` in a set of `Items` (in Any order) (not greedy)
  CountedCheapestInSetSelector : public CountedAnyInSetSelector // Selects X `Items` in a set of items (In cheapest order)
  SingleInSetSelector : public CountedCheapestInSetSelector // Selects Single `Item` in Set
  
  // Looking for as many in a set of items
  GreedyAnyInSetSelector : ManyItemSelector // Select as many X as can be found (in Any order)

### Deal clash

It is likely that some items will be included in more than one deal. This means that we need to find the best deal for the customer.

For example, is it better to buy these items in the Buy 2 Get 1 Free deal, or in the Meal Deal?

To achieve this we have to run through all the permutations of deals and use the Deal permutation that results in the best price.


## Adding new Deals

So long as the deal can be modeled using a multiple of DealSelector, it can be modelled using the current system.
It is proposed to be able to seralise and deserial deals.
This could in future be done with a UI to allow non-tech savvy to easily craft a new deal, which could then be loaded over a network connection.
