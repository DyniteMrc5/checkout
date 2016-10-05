# checkout
A shopping checkout implementation in c++

## Clone, Build and Run
  >git clone --recursive https://github.com/DyniteMrc5/checkout
  
(--recursive needed to also download the googletest submodule)

  >make
  
  >./checkout_test

This will run the test harness which runs a number of tests, some of which output a (fixed width) receipt such as the below.

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
Expressive Deals are called `SmartDeal`.

This is achieved with `Selector`s.

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

    // Looking for many in a set of items
    GreedyAnyInSetSelector : ManyItemSelector // Select as many X as can be found (in Any order)

### Deal clash

It is likely that some items will be included in more than one deal. This means that we need to find the best deal for the customer.

For example, is it better to buy these items in the Buy 2 Get 1 Free deal, or in the Meal Deal?

To achieve this we have to run through all the permutations of deals and use the Deal permutation that results in the best price.


### Adding new Deals

So long as the deal can be modeled using a multiple of DealSelector, it can be modelled using the current system.
It is proposed to be able to seralise and deserial deals. This has been done with the model deal (see below - Legacy Deals): `BuyAofXGetBofYFZ`
This could in future be done with a UI to allow non-tech savvy to easily craft a new deal, which could then be loaded over a network connection.


### Legacy Deals

There are a number of Deals in model_deal.cpp, which do not use the above data structures (SmartDeals). 

They use the same selector and target concept, but do so internally without any `Selector` objects. 
They're not as flexible in this sense, but may be simpler and easier to set up.

### Selectors Appendix

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