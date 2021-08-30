#pragma once
#ifndef CUSTOMER_H_
#define CUSTOMER_H_

#include <vector>
#include <string>
#include "Dish.h"

class Customer
{
  public:
	Customer(std::string c_name, int c_id);
	virtual ~Customer();
	virtual std::vector<int> order(const std::vector<Dish> &menu) = 0;
	virtual std::string toString() const = 0;
	std::string getName() const;
	virtual Customer *Clone() = 0;
	int getId() const;
	bool ordered = false;
	virtual std::string getType() = 0;
	//virtual void ~Customer();
  private:
	const std::string name;
	const int id;
};

#endif
#pragma once
#ifndef VEGETARIANCUSTOMER_H_
#define VEGETARIANCUSTOMER_H_

class VegetarianCustomer : public Customer
{
  public:
	VegetarianCustomer();
	VegetarianCustomer( const VegetarianCustomer &c );
	~VegetarianCustomer();
	Customer* Clone();

	VegetarianCustomer(std::string name, int id);
	std::vector<int> order(const std::vector<Dish> &menu);
	std::string toString() const;
	std::string getType();

  private:
	bool firstOrder = 0;
};

#endif
#pragma once
#ifndef CHEAPCUSTOMER_H_
#define CHEAPCUSTOMER_H_
class CheapCustomer : public Customer
{
  public:
	CheapCustomer(std::string name, int id);
	~CheapCustomer();
	CheapCustomer( const CheapCustomer &c );
	Customer *Clone();
	std::vector<int> order(const std::vector<Dish> &menu);
	std::string toString() const;
	std::string getType();

  private:
	bool firstOrder = 0;
};
#endif
#pragma once
#ifndef SPICYCUSTOMER_H_
#define SPICYCUSTOMER_H_

class SpicyCustomer : public Customer
{
  public:
	SpicyCustomer(std::string name, int id);
	~SpicyCustomer();
	SpicyCustomer( const SpicyCustomer &c );
	Customer *Clone();
	std::vector<int> order(const std::vector<Dish> &menu);
	std::string toString() const;
	std::string getType();

  private:
	bool firstOrder = 0;
};

#endif
#pragma once
#ifndef ALCOHOLICUSTOMER_H_
#define ALCOHOLICCUSTOMER_H_
class AlchoholicCustomer : public Customer
{
  public:
	AlchoholicCustomer(std::string name, int id);
	~AlchoholicCustomer();
	AlchoholicCustomer( const AlchoholicCustomer &c );
	Customer *Clone();
	std::vector<int> order(const std::vector<Dish> &menu);
	std::string toString() const;
	std::string getType();

  private:
	int idNumber;
	std::vector<int> idOrdered;
	bool firstOrder = 0;
};

#endif