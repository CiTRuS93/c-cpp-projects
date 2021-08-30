#pragma once
#ifndef RESTAURANT_H_
#define RESTAURANT_H_

#include <vector>
#include <string>
#include "Dish.h"
#include "Table.h"
#include "Action.h"
#include "Customer.h"

class Restaurant
{
  public:
    Restaurant();
    ~Restaurant();
    Restaurant(Restaurant &res);
    Restaurant(const std::string &configFilePath);
    void start();
    int getNumOfTables() const;
    Table* getTable(int ind);
    const std::vector<BaseAction *> &getActionsLog() const; // Return a reference to the history of actions
    std::vector<Dish> &getMenu();
    Restaurant & operator=( Restaurant &res);
    void clear();

  private:
    std::vector<std::string> getWordVec(std::string str);
    int numOfTables = 0;
    bool open;
    std::vector<Table *> tables;
    std::vector<Dish> menu;
    std::vector<BaseAction *> actionsLog;
    void openTableInRes(std::string str);
    int customerCount = 0;
};

#endif