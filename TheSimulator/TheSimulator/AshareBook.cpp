#include "AshareBook.h"

AshareBook::AshareBook(OrderFactoryPtr orderFactory, TradeFactoryPtr tradeFactory)
	: Book(orderFactory, tradeFactory) { }

void AshareBook::processAgainstTheBuyQueue(const LimitOrderPtr& order, Money minPrice) {

	//test
	// std::cout << "Before:" << std::endl;
	// this->printHuman();

	auto* bestBuyDeque = &m_buyQueue.back();
	while (order->volume() > 0 && bestBuyDeque->price() >= minPrice) {
		LimitOrderPtr iop = bestBuyDeque->front();
		const Volume usedVolume = std::min(iop->volume(), order->volume());
		order->removeVolume(usedVolume);
		iop->removeVolume(usedVolume);
		if(usedVolume > 0) {
			logTrade(OrderDirection::Sell, order->id(), iop->id(), usedVolume, bestBuyDeque->price());
		}
		if (iop->volume() == 0) {
			bestBuyDeque->pop_front();
			unregisterLimitOrder(iop);
		}

		if (bestBuyDeque->empty()) {
			m_buyQueue.pop_back();
			if (m_buyQueue.empty()) {
				break;
			}
			bestBuyDeque = &m_buyQueue.back();
		}
	}

	//test
	// std::cout << "After:" << std::endl;
	// this->printHuman();
}

void AshareBook::processAgainstTheSellQueue(const LimitOrderPtr& order, Money maxPrice) {

	//test
	// std::cout << "Before:" << std::endl;
	// this->printHuman();

	auto* bestSellDeque = &m_sellQueue.front();
	while (order->volume() > 0 && bestSellDeque->price() <= maxPrice) {
		LimitOrderPtr iop = bestSellDeque->front();
		const Volume usedVolume = std::min(iop->volume(), order->volume());
		order->removeVolume(usedVolume);
		iop->removeVolume(usedVolume);
		if (usedVolume > 0) {
			logTrade(OrderDirection::Buy, order->id(), iop->id(), usedVolume, bestSellDeque->price());
		}
		if (iop->volume() == 0) {
			bestSellDeque->pop_front();
			unregisterLimitOrder(iop);
		}

		if (bestSellDeque->empty()) {
			m_sellQueue.pop_front();
			if (m_sellQueue.empty()) {
				break;
			}
			bestSellDeque = &m_sellQueue.front();
		}
	}

	//test
	// std::cout << "After:" << std::endl;
	// this->printHuman();
}

void AshareBook::processAgainstTheBuyQueue(const KlevelMarketOrderPtr& order, Money minPrice) {

	//test
	std::cout << "Before:" << std::endl;
	std::cout << order->volume() << std::endl;
	this->printHuman();

	auto* bestBuyDeque = &m_buyQueue.back();
	while (order->volume() > 0 && bestBuyDeque->price() >= minPrice && order->K_level() > 0) {
		LimitOrderPtr iop = bestBuyDeque->front();
		const Volume usedVolume = std::min(iop->volume(), order->volume());
		order->removeVolume(usedVolume);
		iop->removeVolume(usedVolume);
		if(usedVolume > 0) {
			logTrade(OrderDirection::Sell, order->id(), iop->id(), usedVolume, bestBuyDeque->price());
		}
		if (iop->volume() == 0) {
			bestBuyDeque->pop_front();
			unregisterLimitOrder(iop);
		}

		if (bestBuyDeque->empty()) {
			m_buyQueue.pop_back();
			if (m_buyQueue.empty()) {
				break;
			}
			bestBuyDeque = &m_buyQueue.back();
			order->decreaseLevel();
			std::cout << "decrease level: " << order->K_level() << std::endl;
			std::cout << order->volume() << std::endl;
		}
	}

	//test
	std::cout << "After:" << std::endl;
	std::cout << order->volume() << std::endl;
	this->printHuman();
}

void AshareBook::processAgainstTheSellQueue(const KlevelMarketOrderPtr& order, Money maxPrice) {

	//test
	std::cout << "Before:" << std::endl;
	std::cout << order->volume() << std::endl;
	this->printHuman();

	auto* bestSellDeque = &m_sellQueue.front();
	while (order->volume() > 0 && bestSellDeque->price() <= maxPrice && order->K_level() > 0) {
		LimitOrderPtr iop = bestSellDeque->front();
		const Volume usedVolume = std::min(iop->volume(), order->volume());
		order->removeVolume(usedVolume);
		iop->removeVolume(usedVolume);
		if (usedVolume > 0) {
			logTrade(OrderDirection::Buy, order->id(), iop->id(), usedVolume, bestSellDeque->price());
		}
		if (iop->volume() == 0) {
			bestSellDeque->pop_front();
			unregisterLimitOrder(iop);
		}

		if (bestSellDeque->empty()) {
			m_sellQueue.pop_front();
			if (m_sellQueue.empty()) {
				break;
			}
			bestSellDeque = &m_sellQueue.front();
			order->decreaseLevel();
			std::cout << "decrease level: " << order->K_level() << std::endl;
			std::cout << order->volume() << std::endl;
		}
	}

	//test
	std::cout << "After:" << std::endl;
	std::cout << order->volume() << std::endl;
	this->printHuman();
}

void AshareBook::processAgainstTheSellQueue(const OrderPtr& order, Money maxPrice) {
	std::cout << typeid(order).name() << std::endl;
}
void AshareBook::processAgainstTheBuyQueue(const OrderPtr& order, Money minPrice) {}