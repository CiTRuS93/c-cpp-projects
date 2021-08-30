#pragma once
#ifndef ACTION_H_
#define ACTION_H_

#include <string>
#include <iostream>
#include "Customer.h"

 enum ActionStatus {
	PENDING, COMPLETED, ERROR
};

//Forward declaration
class Restaurant;
#pragma once
#ifndef BASEACTION_H_
#define BASEACTION_H_
////============================= BaseAction =====================================/////

class BaseAction {
public:
	BaseAction();
	BaseAction( const BaseAction &b );
	virtual ~BaseAction();
	void setStatus(ActionStatus a);
	ActionStatus getStatus() const;
	virtual void act( Restaurant& restaurant ) = 0;
	virtual std::string toString() const = 0;
	virtual BaseAction* Clone() = 0;
	std::string statusTostring() const;
protected:
	void complete();
	void error(std::string errorMsg);
	std::string getErrorMsg() const;
	std::string orderStatus;
private:
	std::string errorMsg;
	ActionStatus status;
	
};

#endif
#pragma once
#ifndef OPENTABLE_H_
#define OPENTABLE_H_
////============================= OpenTable =====================================/////

class OpenTable : public BaseAction {
public:
	OpenTable( int id, std::vector<Customer *> &customersList );
	OpenTable( const OpenTable &b );
	~OpenTable();
	BaseAction* Clone();
	void act( Restaurant &restaurant );
	std::string toString() const;
private:
	const int tableId;
	const std::vector<Customer *> customers;
};

#endif
#pragma once
#ifndef ORDER_H_
#define ORDER_H_
////============================= Order =====================================/////

class Order : public BaseAction {
public:
	Order( int id );
	Order( const Order& o );
	~Order();
	BaseAction* Clone();
	void act( Restaurant &restaurant );
	std::string toString() const;
private:
	const int tableId;
};
#endif

#pragma once
#ifndef MOVECUSTOMER_H_
#define MOVECUSTOMER_H_
////============================= MoveCustomer =====================================/////

class MoveCustomer : public BaseAction {
public:
	MoveCustomer( int src, int dst, int customerId );
	MoveCustomer( const MoveCustomer &m);
	~MoveCustomer();
	BaseAction* Clone();
	void act( Restaurant &restaurant );
	std::string toString() const;
private:
	const int srcTable;
	const int dstTable;
	const int id;
};

#endif
#pragma once
#ifndef CLOSE_H_
#define CLOSE_H_
////============================= Close =====================================/////

class Close : public BaseAction {
public:
	Close( int id );
	Close ( const Close &c );
	~Close();
	BaseAction* Clone();
	void act( Restaurant &restaurant );
	std::string toString() const;
private:
	const int tableId;
};

#endif
#pragma once
#ifndef CLOSEALL_H_
#define CLOSEALL_H_
////============================= CloseAll =====================================/////

class CloseAll : public BaseAction {
public:
	CloseAll();
	~CloseAll();
	void act( Restaurant &restaurant );
	std::string toString() const;
private:
};
#endif
#pragma once
#ifndef PRINTMENU_H_
#define PRINTMENU_H_
////============================= PrintMenu =====================================/////

class PrintMenu : public BaseAction {
public:
	PrintMenu();
	PrintMenu( const PrintMenu &p );
	~PrintMenu();
	BaseAction* Clone();
	void act( Restaurant &restaurant );
	std::string toString() const;
private:
};
#endif
#pragma once
#ifndef PRINTTABLESTATUS_H_
#define PRINTTABLESTATUS_H_
////============================= PrintTableStatus =====================================/////

class PrintTableStatus : public BaseAction {
public:
	PrintTableStatus( int id );
	PrintTableStatus ( const PrintTableStatus &p );
	~PrintTableStatus();
	BaseAction* Clone();
	void act( Restaurant &restaurant );
	std::string toString() const;
private:
	const int tableId;
};
#endif
#pragma once
#ifndef PRINTACTIONSLOG_H_
#define PRINTACTIONSLOG_H_
////============================= PrintActionsLog =====================================/////

class PrintActionsLog : public BaseAction {
public:
	PrintActionsLog();
	PrintActionsLog( const PrintActionsLog &p );
	~PrintActionsLog();
	BaseAction* Clone();
	void act( Restaurant &restaurant );
	std::string toString() const;
private:
};

#endif
#pragma once
#ifndef BACKUPRESTAURANT_H_
#define BACKUPRESTAURANT_H_
////============================= BackupRestaurant =====================================/////

class BackupRestaurant : public BaseAction {
public:
	BackupRestaurant();
	BackupRestaurant( const BackupRestaurant &b );
	~BackupRestaurant();
	BaseAction* Clone();
	void act( Restaurant &restaurant );
	std::string toString() const;
private:
};
#endif
#pragma once
#ifndef RESTORERESTAURANT_H_
#define RESTORERESTAURANT_H_
////============================= RestoreResturant =====================================/////

class RestoreResturant : public BaseAction {
public:
	RestoreResturant();
	RestoreResturant( const RestoreResturant &r );
	~RestoreResturant();
	BaseAction* Clone();
	void act( Restaurant &restaurant );
	std::string toString() const;

};


#endif
#endif