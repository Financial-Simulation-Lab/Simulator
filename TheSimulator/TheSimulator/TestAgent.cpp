#include "TestAgent.h"

#include "Simulation.h"
#include "SimulationException.h"
#include "ExchangeAgentMessagePayloads.h"
#include "ParameterStorage.h"

#include <random>
#include <cmath>
#include <iostream>
#include <regex>
#include <ctime>

using namespace std;

TestAgent::TestAgent(const Simulation* simulation)
	: Agent(simulation), m_exchange(""), m_file("") {}

TestAgent::TestAgent(const Simulation* simulation, const std::string& name)
	: Agent(simulation, name), m_exchange(""), m_file("") {}

void TestAgent::configure(const pugi::xml_node& node, const std::string& configurationPath) {
	Agent::configure(node, configurationPath);

	pugi::xml_attribute att;
	if (!(att = node.attribute("exchange")).empty()) {
		m_exchange = simulation()->parameters().processString(att.as_string());
	}

	if (!(att = node.attribute("file")).empty()) {
		m_file = att.as_string();
		inFile.open(m_file, ios::in);
    	std::string line;
		getline(inFile, line);
	}

}

void Stringsplit(const string& str, const string& split, vector<string>& res) {
	//std::regex ws_re("\\s+"); // 正则表达式,匹配空格 
	std::regex reg(split);		// 匹配split
	std::sregex_token_iterator pos(str.begin(), str.end(), reg, -1);
	decltype(pos) end;              // 自动推导类型 
	for (; pos != end; ++pos) {
		res.push_back(pos->str());
	}
}

void TestAgent::receiveMessage(const MessagePtr& msg) {
	const Timestamp currentTimestamp = simulation()->currentTimestamp();
    std::string line;
	std::string delimiter = ",";
	double time, price;
	int volume, order_type, market_type;
	OrderDirection direction;

	if (msg->type == "EVENT_SIMULATION_START") {
        if (getline(inFile, line)) {
		   	vector<string> strList;
		   	Stringsplit(line, delimiter, strList);

			time = stod(strList.at(8));
			price = stod(strList.at(1));
			volume = stoi(strList.at(2));
			direction = stoi(strList.at(3)) == 1 ? OrderDirection::Buy : OrderDirection::Sell;
			order_type = stoi(strList.at(4));

			if (time < 3) {
				simulation()->dispatchMessage(time, 0, this->name(), this->name(), "CONTINURE", std::make_shared<EmptyPayload>());
				return;
			}

			if (order_type == 2) {
				auto pptr = std::make_shared<PlaceOrderLimitPayload>(direction, volume, price);
				simulation()->dispatchMessage(time, 0, this->name(), m_exchange, "PLACE_ORDER_LIMIT", pptr);
			}

			else {
				Level level = stoi(strList.at(6));
				auto pptr = std::make_shared<PlaceKlevelOrderMarketPayload>(direction, volume, level);
				simulation()->dispatchMessage(time, 0, this->name(), m_exchange, "PLACE_Klevel_ORDER_MARKET", pptr);
			}
        }
	}
	else {
		if (getline(inFile, line)) {
		   	vector<string> strList;
		   	Stringsplit(line, delimiter, strList);

			time = stod(strList.at(8));
			price = stod(strList.at(1));
			volume = stoi(strList.at(2));
			direction = stoi(strList.at(3)) == 1 ? OrderDirection::Buy : OrderDirection::Sell;
			order_type = stoi(strList.at(4));
			// std::cout << time << " " << price << " " << (direction == OrderDirection::Buy) << std::endl;

			if (time < 3) {
				simulation()->dispatchMessage(time, 0, this->name(), this->name(), "CONTINURE", std::make_shared<EmptyPayload>());
				return;
			}

			if (order_type == 2) {
				auto pptr = std::make_shared<PlaceOrderLimitPayload>(direction, volume, price);
				simulation()->dispatchMessage(time, 0, this->name(), m_exchange, "PLACE_ORDER_LIMIT", pptr);
			}

			else {
				Level level = stoi(strList.at(6));

				std::cout << "Level: " << level << std::endl;
				
				auto pptr = std::make_shared<PlaceKlevelOrderMarketPayload>(direction, volume, level);
				simulation()->dispatchMessage(time, 0, this->name(), m_exchange, "PLACE_Klevel_ORDER_MARKET", pptr);
			}
        }
	}

}