#pragma once

#include <deque>

#include "Book.h"

class AshareBook : public Book {
public:
	AshareBook(OrderFactoryPtr orderRecordPtr, TradeFactoryPtr tradeRecordPtr);
protected:
	void processAgainstTheBuyQueue(const LimitOrderPtr& order, Money minPrice) override;
	void processAgainstTheSellQueue(const LimitOrderPtr& order, Money maxPrice) override;
	void processAgainstTheBuyQueue(const KlevelMarketOrderPtr& order, Money minPrice) override;
	void processAgainstTheSellQueue(const KlevelMarketOrderPtr& order, Money maxPrice) override;

	void processAgainstTheSellQueue(const OrderPtr& order, Money maxPrice) override;
	void processAgainstTheBuyQueue(const OrderPtr& order, Money minPrice) override;
};

