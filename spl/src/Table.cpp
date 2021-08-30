
#include "Table.h"
#include "Customer.h"
#include "Dish.h"
#include <iostream>
#include <typeinfo>

typedef std::pair<int, Dish> OrderPair;

//A list of pairs for each order in a table - (customer_id, Dish)

//Vegetarian � VEG, Beverage � BVG, Alcoholic � ALC, Spicy �SPC

Table::Table(int t_capacity)
{
	capacity = t_capacity;
	currentCapacity = 0;
}

Table::Table(const Table *t) : currentCapacity(t->currentCapacity), capacity(t->getCapacity()), open(t->isOpen())
{
	for (int i = 0; i < (int)t->customersList.size(); i++)
	{
		this->customersList.push_back(t->customersList[i]->Clone());
	}

	std::vector<OrderPair> ol = t->orderList;

	for (int i = 0; (unsigned)i < ol.size(); i++)
	{
		int j = ol[i].first;
		Dish d(ol[i].second);
		OrderPair p = std::make_pair(j, d);
		this->orderList.push_back(p);
	}

	ol.clear();
}

Table::~Table()
{

	orderList.clear();
}

int Table::getCapacity() const
{
	return capacity;
}

void Table::addCustomer(Customer *customer)
{
	if (currentCapacity < capacity)
	{
		customersList.push_back(customer);
		currentCapacity++;
	}
}

void Table::removeCustomer(int id)
{
	if (customersList.empty())
	{
		open = false;
	}
	int size = customersList.size();
	for (int i = 0; i < size; i++)
	{
		if (customersList[i]->getId() == id)
		{
			customersList.erase(customersList.begin() + i);
			i = size;
		}
	}
	std::vector<OrderPair> tempOrder;
	for (int i = 0; i < (int)orderList.size(); i++)
	{
		if (orderList[i].first != id)
		{
			tempOrder.push_back(orderList[i]);
		}
	}
	orderList.clear();
	for (int i = 0; i < (int)tempOrder.size(); i++)
	{
		orderList.push_back(tempOrder[i]);
	}

	size = orderList.size();

	currentCapacity--;
	if (currentCapacity == 0)
	{
		closeTable();
	}
}

void Table::addDishes(const int id, std::vector<OrderPair> &dishes)
{

	for (int i = 0; i < (int)dishes.size(); i++)
	{
		if (dishes[i].first == id)
		{
			OrderPair p = std::make_pair(dishes[i].first, dishes[i].second);
			orderList.push_back(p);
		}
	}
}
Customer *Table::getCustomer(int id)
{
	int size = customersList.size();
	for (int i = 0; i < size; i++)
	{
		if (customersList[i]->getId() == id)
			return customersList[i];
	}
	return nullptr;
}

std::vector<Customer *> &Table::getCustomers()
{
	return customersList;
}

std::vector<OrderPair> &Table::getOrders()
{
	return orderList;
}

void Table::order(const std::vector<Dish> &menu)
{
	for (int i = 0; i < currentCapacity; i++)
	{
		std::vector<int> singleOrder = customersList[i]->order(menu);
		for (int j = 0; (unsigned)j < singleOrder.size(); j++)
		{
			if (singleOrder[j] != -1)
			{
				OrderPair p = std::make_pair(customersList[i]->getId(), menu[singleOrder[j]]);
				orderList.push_back(p);
				std::cout << customersList[i]->getName() << " ordered " << p.second.getName() << "\n";
			}
		}
	}
}

void Table::openTable()
{
	open = true;
}

void Table::closeTable()
{

	for (int i = 0; i < (int)customersList.size(); i++)
	{
		delete customersList[i];
	}
	while (customersList.size() > 0)
	{
		customersList.erase(customersList.begin());
	}

	currentCapacity = 0;
	open = false;
}

int Table::getBill()
{
	int size = orderList.size();
	int sum = 0;
	for (int i = 0; i < size; i++)
	{
		sum += orderList[i].second.getPrice();
	}
	return sum;
}

bool Table::isOpen() const
{
	return open;
}
bool Table::isFull()
{
	return (currentCapacity == capacity);
}
Table &Table::operator=(Table &t)
{
	this->capacity = t.getCapacity();
	this->customersList = t.getCustomers();
	this->currentCapacity = t.currentCapacity;
	this->open = t.isOpen();
	std::vector<OrderPair> ol = t.orderList;

	for (int i = 0; (unsigned)i < ol.size(); i++)
	{
		int j = ol[i].first;
		Dish d(ol[i].second);
		OrderPair p = std::make_pair(j, d);
		this->orderList.push_back(p);
	}
	return *this;
}