
#include <string>
#include <iostream>
#include "Customer.h"
#include "Table.h"
#include "Restaurant.h"
#include <vector>
#include "Dish.h"
#include "Action.h"
extern Restaurant *backup;

////============================= BaseAction =====================================/////
BaseAction::BaseAction() : status(PENDING)
{
}
BaseAction::BaseAction(const BaseAction &b)
{
	this->errorMsg = b.errorMsg;
	this->status = b.status;
	orderStatus = "";
}
BaseAction::~BaseAction()
{
}
ActionStatus BaseAction::getStatus() const
{
	return status;
}
void BaseAction::error(std::string errorMsg)
{
	this->errorMsg = errorMsg;
}
std::string BaseAction::getErrorMsg() const
{
	if (errorMsg != "")
	{
		return ": " + errorMsg;
	}
	return "";
}
void BaseAction::setStatus(ActionStatus a)
{
	this->status = a;
}
std::string BaseAction::statusTostring() const
{
	switch (this->getStatus())

	{
	case ERROR:
		return "Error";
		break;
	case COMPLETED:
		return "Completed";
		break;
	default:
		return "Pending";
		break;
	}
}
////============================= OpenTable =====================================/////
OpenTable::OpenTable(int id, std::vector<Customer *> &customersList) : BaseAction(), tableId(id), customers(customersList)
{
}

OpenTable::OpenTable(const OpenTable &b) : BaseAction(b), tableId(b.tableId), customers(b.customers)
{
}

OpenTable::~OpenTable()
{
	if (this->getStatus() == ERROR)
	{
		for (int i = 0; i < (int)customers.size() && customers[i] != NULL; i++)
		{
			delete (customers[i]);
		}
	}
	for (int i = 0; 0 < (int)customers.size() && customers[i] != nullptr; i++)
	{
		delete (customers[i]);
	}
}

BaseAction *OpenTable::Clone()
{
	std::vector<Customer *> cust;
	for (int i = 0; i < (int)customers.size(); i++)
	{
		cust.push_back(customers[i]->Clone());
	}
	OpenTable *a = new OpenTable(this->tableId, cust);
	a->setStatus(this->getStatus());
	a->error(this->getErrorMsg());
	return a;
}

void OpenTable::act(Restaurant &restaurant)
{
	if (tableId <= restaurant.getNumOfTables() && !(restaurant.getTable(tableId)->isOpen()))
	{
		restaurant.getTable(tableId)->openTable();
		for (int i = 0; (unsigned)i < customers.size(); i++)
		{
			restaurant.getTable(tableId)->addCustomer(customers[i]);
		}
		this->setStatus(COMPLETED);
	}
	else
	{
		this->setStatus(ERROR);
		this->error("Table does not exist or is already open");
		std::cout << this->statusTostring() << this->getErrorMsg() << std::endl;
	}
}

std::string OpenTable::toString() const
{
	std::string str = "";
	str += "open " + std::to_string(tableId) + " ";

	for (int i = 0; i < (int)customers.size(); i++)
	{
		str += customers[i]->getName() + "," + customers[i]->getType() + " ";
	}
	str += this->statusTostring() + this->getErrorMsg();
	return str;
}
////============================= Order =====================================/////
Order::Order(int id) : tableId(id)
{
}
Order::Order(const Order &o) : BaseAction(o), tableId(o.tableId)
{
}
Order::~Order()
{
}

BaseAction *Order::Clone()
{
	return new Order(*this);
}

void Order::act(Restaurant &restaurant)
{
	restaurant.getTable(tableId)->order(restaurant.getMenu());
	this->setStatus(COMPLETED);
}

std::string Order::toString() const
{
	return "order " + std::to_string(tableId) + " " + this->statusTostring() + this->getErrorMsg();
}
////============================= MoveCustomer =====================================/////
MoveCustomer::MoveCustomer(int src, int dst, int customerId) : srcTable(src), dstTable(dst), id(customerId)
{
}

MoveCustomer::MoveCustomer(const MoveCustomer &m) : BaseAction(m), srcTable(m.srcTable), dstTable(m.dstTable), id(m.id)
{
}
MoveCustomer::~MoveCustomer()
{
}

BaseAction *MoveCustomer::Clone()
{
	return new MoveCustomer(*this);
}

void MoveCustomer::act(Restaurant &restaurant)
{
	if (!restaurant.getTable(dstTable)->isFull() && restaurant.getTable(dstTable)->isOpen())
	{
		Customer *cu = restaurant.getTable(srcTable)->getCustomer(id);
		restaurant.getTable(dstTable)->addCustomer(cu);
		restaurant.getTable(dstTable)->addDishes(id, restaurant.getTable(srcTable)->getOrders());
		restaurant.getTable(srcTable)->removeCustomer(id);

		this->setStatus(COMPLETED);
	}
	else
	{
		this->setStatus(ERROR);
		this->error("Cannot move customer");
		std::cout << "Error: Cannot move customer" << std::endl;
	}
}
std::string MoveCustomer::toString() const
{
	return "move " + std::to_string(srcTable) + " " + std::to_string(dstTable) + " " + std::to_string(id) + " " + this->statusTostring() + this->getErrorMsg();
}
////============================= Close =====================================/////
Close::Close(int id) : BaseAction(), tableId(id)
{
}

Close::Close(const Close &c) : BaseAction(c), tableId(c.tableId)
{
}

Close::~Close()
{
}

BaseAction *Close::Clone()
{
	return new Close(*this);
}

void Close::act(Restaurant &restaurant)
{
	if (restaurant.getTable(tableId)->isOpen())
	{
		std::cout << "Table " << tableId << " was closed. Bill " << restaurant.getTable(tableId)->getBill() << "NIS\n";
		restaurant.getTable(tableId)->closeTable();
		this->setStatus(COMPLETED);
	}
	else
	{
		this->error("Table is already closed");
		this->setStatus(ERROR);
	}
}

std::string Close::toString() const
{
	std::string statStr = this->statusTostring();
	return statStr + " " + this->getErrorMsg() + "\n";
}

////============================= CloseAll =====================================/////
CloseAll::CloseAll()
{
}

CloseAll::~CloseAll()
{
}

void CloseAll::act(Restaurant &restaurant)
{
	for (int i = 0; i < restaurant.getNumOfTables(); i++)
	{
		restaurant.getTable(i)->closeTable();
	}
}

std::string CloseAll::toString() const
{
	return "";
}

////============================= PrintMenu =====================================/////
PrintMenu::PrintMenu()
{
}

PrintMenu::PrintMenu(const PrintMenu &p) : BaseAction(p)
{
}

PrintMenu::~PrintMenu()
{
}

BaseAction *PrintMenu::Clone()
{
	return new PrintMenu(*this);
}

void PrintMenu::act(Restaurant &restaurant)
{
	std::vector<Dish> men = restaurant.getMenu();
	for (int i = 0; (unsigned)i < men.size(); i++)
	{
		men[i].toString();
	}
}
std::string PrintMenu::toString() const
{
	return "";
}
////============================= PrintTableStatus =====================================/////
PrintTableStatus::PrintTableStatus(int id) : tableId(id)
{
}

PrintTableStatus::PrintTableStatus(const PrintTableStatus &p) : BaseAction(p), tableId(p.tableId)
{
}

PrintTableStatus::~PrintTableStatus()
{
}

BaseAction *PrintTableStatus::Clone()
{
	return new PrintTableStatus(*this);
}

void PrintTableStatus::act(Restaurant &restaurant)
{
	Table *t = restaurant.getTable(tableId);
	if (t->isOpen())
	{ // table is open
		std::cout << "Table " << tableId << " status: open" << std::endl;
		std::cout << "Customers:" << std::endl;
		std::vector<Customer *> customers = t->getCustomers();
		for (int i = 0; (unsigned)i < customers.size(); i++)
		{ // prints the customers of the table
			std::cout << customers[i]->toString() << std::endl;
		}
		std::cout << "Orders:" << std::endl;
		std::vector<OrderPair> orders = t->getOrders();
		for (int i = 0; (unsigned)i < orders.size(); i++)
		{ // prints the orders of the table
			std::cout << orders[i].second.getName() << " " << orders[i].second.getPrice() << "NIS " << orders[i].first << std::endl;
		}
		std::cout << "Current Bill: " << t->getBill() << "NIS " << std::endl;
	}
	else
	{ // table is cloesd
		std::cout << "Table " << tableId << " status: closed" << std::endl;
	}
	this->setStatus(COMPLETED);
}
std::string PrintTableStatus::toString() const
{
	return "status " + std::to_string(tableId) + " " + this->statusTostring() + this->getErrorMsg();
}
////============================= PrintActionsLog =====================================/////
PrintActionsLog::PrintActionsLog()
{
}

PrintActionsLog::PrintActionsLog(const PrintActionsLog &p) : BaseAction(p)
{
}

PrintActionsLog::~PrintActionsLog()
{
}

BaseAction *PrintActionsLog::Clone()
{
	return new PrintActionsLog(*this);
}

void PrintActionsLog::act(Restaurant &restaurant)
{
	std::vector<BaseAction *> Log = restaurant.getActionsLog();

	for (int i = 0; (unsigned)i < Log.size(); i++)
	{
		std::cout << Log[i]->toString() << "\n";
	}
	this->setStatus(COMPLETED);
}
std::string PrintActionsLog::toString() const
{
	return "";
}
////============================= BackupRestaurant =====================================/////
BackupRestaurant::BackupRestaurant()
{
}

BackupRestaurant::BackupRestaurant(const BackupRestaurant &b) : BaseAction(b)
{
}

BackupRestaurant::~BackupRestaurant()
{
}

BaseAction *BackupRestaurant::Clone()
{
	return new BackupRestaurant(*this);
}

void BackupRestaurant::act(Restaurant &restaurant)
{
	backup = new Restaurant(restaurant);
	this->setStatus(COMPLETED);
}

std::string BackupRestaurant::toString() const
{
	return "backup " + this->statusTostring();
}
////============================= RestoreResturant =====================================/////
RestoreResturant::RestoreResturant()
{
}

RestoreResturant::RestoreResturant(const RestoreResturant &r) : BaseAction(r)
{
}

RestoreResturant::~RestoreResturant()
{
}

BaseAction *RestoreResturant::Clone()
{
	return new RestoreResturant(*this);
}

void RestoreResturant::act(Restaurant &restaurant)
{
	if (backup != nullptr)
	{

		restaurant.clear();
		restaurant = *backup;

		this->setStatus(COMPLETED);
	}
	else
	{
		this->setStatus(ERROR);
		this->error("No backup");
	}
}
std::string RestoreResturant::toString() const
{
	return "restore " + this->statusTostring();
}
