
all: checkout_test

clean: 
	rm -f checkout.o
	rm -f selectors.o
	rm -f deal.o
	rm -f model_deal.o
	rm -f checkout_test.o
	rm -f checkout_test

checkout:
	echo "Make checkout.o"
	g++ -g --std=c++11 -c checkout.cpp -o checkout.o

selectors:
	echo "Making selectors.o"
	g++ -g --std=c++11 -c selectors.cpp -o selectors.o

deal:
	echo "Making deal.o"
	g++ -g --std=c++11 -c model_deal.cpp -o model_deal.o
	g++ -g --std=c++11 -c deal.cpp -o deal.o
	
checkout_test_o: checkout deal
	echo "Make checkout_test.o"
	g++ -g -Igoogletest/googletest/include --std=c++11 checkout.o -c checkout_test.cpp -o checkout_test.o

regenerate_gtest_main:
	$(MAKE) -C googletest/googletest/make all

checkout_test: selectors deal checkout checkout_test_o regenerate_gtest_main
	echo "Make checkout_test"
	g++ -isystem -Igoogletest/googletest/include -g -Wall -Wextra -pthread \
		-lpthread googletest/googletest/make/gtest_main.a checkout_test.o checkout.o deal.o model_deal.o selectors.o -o checkout_test
