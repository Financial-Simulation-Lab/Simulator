#include "Order.h"

#include <iostream>
#include <fstream>
#include <string>

void BasicOrder::printHuman() const {
	std::cout << m_id << ":\t" << m_timestamp << "\t" << m_volume;
}

std::string BasicOrder::printCSV() const {
	// std::cout << m_id << "," << m_timestamp << "," << m_volume;

	return std::to_string(m_id) + "," + std::to_string(m_timestamp) + "," + std::to_string(m_volume); 
}

BasicOrder::BasicOrder(OrderID id, Timestamp timestamp, Volume orderVolume)
	: m_id(id), m_timestamp(timestamp), m_volume(orderVolume) { }

Order::Order(const Order& order)
	: BasicOrder(order), m_direction(order.m_direction) { }

Order::Order(Order&& order)
	: BasicOrder(order), m_direction(order.m_direction)  { }

void Order::printHuman() const {
	this->BasicOrder::printHuman();

	std::cout << "\t" << (m_direction == OrderDirection::Buy ? "buy" : "sell");
}

std::string Order::printCSV() const {
	// this->BasicOrder::printCSV();

	// std::cout << "," << (m_direction == OrderDirection::Buy ? "buy" : "sell");

	return this->BasicOrder::printCSV() + "," + (m_direction == OrderDirection::Buy ? "buy" : "sell");
}

Order::Order(OrderID id, OrderDirection direction, Timestamp timestamp, Volume volume)
	: BasicOrder(id, timestamp, volume), m_direction(direction) {
}

void MarketOrder::printHuman() const { 
	this->Order::printHuman();

	std::cout << "\tMKT" << std::endl; // note this, outputting just cents
}

std::string MarketOrder::printCSV() const { 
	// this->BasicOrder::printCSV();

	// std::cout << ",MKT" << std::endl;

	return this->Order::printCSV() + ",MKT\n";
}

MarketOrder::MarketOrder(OrderID id, OrderDirection direction, Timestamp timestamp, Volume volume)
	: Order(id, direction, timestamp, volume) {
	
}

LimitOrder::LimitOrder(OrderID id, OrderDirection direction, Timestamp timestamp, Volume volume, const Money& price)
	: Order(id, direction, timestamp, volume), m_price(price) {
}

void LimitOrder::printHuman() const {
	this->Order::printHuman();

	std::cout << "\tLMT\t" << m_price.toCentString() << std::endl; // note this, outputting just cents
}

std::string LimitOrder::printCSV() const {
	// this->BasicOrder::printCSV();

	// std::cout << ",LMT," << m_price.toFullString() << std::endl;

	return this->Order::printCSV() + ",LMT," + m_price.toFullString() + "\n";
}
