#include <vector>
#include "Dish.h"
#include "Table.h"
#include "Action.h"
#include "test.h"
#include "Customer.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Restaurant.h"
using namespace std;
extern Restaurant *backup;
int stringToInt(string str);

Restaurant::Restaurant(Restaurant &res)
{
    for (int i = 0; i < (int)res.actionsLog.size(); i++)
    {
        BaseAction *a = res.actionsLog[i]->Clone();
        this->actionsLog.push_back(a);
    }
    this->customerCount = res.customerCount;
    std::vector<Dish> me = res.getMenu();
    for (int i = 0; (unsigned)i < me.size(); i++)
    {
        Dish d(&me[i]);
        this->menu.push_back(d);
    }
    me.clear();
    this->numOfTables = res.numOfTables;
    this->open = res.open;

    for (int i = 0; i < res.numOfTables; i++)
    {
        Table *temp = res.getTable(i);
        Table *t = new Table(temp);
        this->tables.push_back(t);
    }
}

Restaurant::Restaurant() {}

Restaurant::Restaurant(const std::string &configFilePath)
{

    string freader = "";
    int dishId = 0;
    vector<int> capacitys;
    ifstream resFile(configFilePath);
    if (resFile.is_open())
    {
        while (getline(resFile, freader))
        {

            if (freader[0] != '#' && !freader.empty() && numOfTables == 0)
            {
                numOfTables = (int)freader[0] - 48;
                for (int num = 0; num < numOfTables; num++)
                {
                    capacitys.push_back(0);
                }
            }
            else if (freader[0] != '#' && !freader.empty() && capacitys[0] == 0)
            {
                int index = 0;
                for (int i = 0; i <= (int)freader.length(); i++)
                {
                    if (freader[i] != ',' && freader[i] != '\r' && i < (int)freader.length())
                    {
                        capacitys[index] = capacitys[index] * 10 + ((int)freader[i] - 48);
                    }
                    else
                    {
                        if (index < numOfTables)
                        {
                            Table *t = new Table((int)capacitys[index]);
                            tables.push_back(t);
                            t = nullptr;
                        }
                        index++;
                    }
                }
            }
            else if (freader[0] != '#' && !freader.empty())
            {
                int startChar[3];
                int endChar[3];
                int price = 0;
                int index = 0;
                for (int i = 0; i < 3; i++)
                {

                    startChar[i] = index;
                    while (freader[index] != ',' && freader[index] != '\r' && index < (int)freader.length())
                    {

                        index++;
                    }
                    endChar[i] = index;
                    index++;
                }
                for (int i = 0; i < endChar[2] - startChar[2]; i++)
                {
                    price = price * 10 + freader[i + startChar[2]] - 48;
                }
                // Dishes
                if (freader.substr(startChar[1], endChar[1] - startChar[1]) == "VEG")
                {

                    Dish dish(dishId, freader.substr(startChar[0], endChar[0]), price, VEG);
                    menu.push_back(dish);
                    dishId++;
                }
                else if (freader.substr(startChar[1], endChar[1] - startChar[1]) == "SPC")
                {

                    Dish dish(dishId, freader.substr(startChar[0], endChar[0]), price, SPC);
                    menu.push_back(dish);
                    dishId++;
                }
                else if (freader.substr(startChar[1], endChar[1] - startChar[1]) == "BVG")
                {

                    Dish dish(dishId, freader.substr(startChar[0], endChar[0]), price, BVG);
                    menu.push_back(dish);
                    dishId++;
                }
                else
                {

                    Dish dish(dishId, freader.substr(startChar[0], endChar[0]), price, ALC);
                    menu.push_back(dish);
                    dishId++;
                }
            }
        }

        resFile.close();
    }
    else
    {
        cout << "cant open";
    }
}

void Restaurant::start()
{
    std::cout << "Restaurant is now open!" << std::endl;
    std::string str = "";

    while (str != "closeall")
    {
        getline(std::cin, str);
        vector<string> strVec = getWordVec(str);
        if (strVec[0] == "open")
        {
            int tableId = stringToInt(strVec[1]);
            std::vector<Customer *> custList;
            for (int i = 2; (unsigned)i < strVec.size(); i += 2)
            {

                if (strVec[i + 1] == "veg")
                {

                    VegetarianCustomer *v = new VegetarianCustomer(strVec[i], customerCount);
                    custList.push_back(v);
                }
                if (strVec[i + 1] == "chp")
                {

                    CheapCustomer *v = new CheapCustomer(strVec[i], customerCount);
                    custList.push_back(v);
                }
                if (strVec[i + 1] == "spc")
                {

                    SpicyCustomer *v = new SpicyCustomer(strVec[i], customerCount);
                    custList.push_back(v);
                }
                if (strVec[i + 1] == "alc")
                {

                    AlchoholicCustomer *v = new AlchoholicCustomer(strVec[i], customerCount);
                    custList.push_back(v);
                }
                customerCount++;
            }
            OpenTable *o = new OpenTable(tableId, custList);
            o->act(*this);
            actionsLog.push_back(o);
        }
        else if (strVec[0] == "order")
        {
            Order *o = new Order(stringToInt(strVec[1]));
            o->act(*this);
            actionsLog.push_back(o);
        }
        else if (strVec[0] == "move")
        {
            MoveCustomer *mc = new MoveCustomer(stringToInt(strVec[1]), stringToInt(strVec[2]), stringToInt(strVec[3]));
            mc->act(*this);
            actionsLog.push_back(mc);
        }
        else if (strVec[0] == "close")
        {
            Close *c = new Close(stringToInt(strVec[1]));
            c->act(*this);
            actionsLog.push_back(c);
        }
        else if (strVec[0] == "menu")
        {
            PrintMenu *pm = new PrintMenu();
            pm->act(*this);
            actionsLog.push_back(pm);
        }
        else if (strVec[0] == "status")
        {
            PrintTableStatus *pts = new PrintTableStatus(stringToInt(strVec[1]));
            pts->act(*this);
            actionsLog.push_back(pts);
        }
        else if (strVec[0] == "log")
        {
            PrintActionsLog *pal = new PrintActionsLog();
            pal->act(*this);

            delete pal;
        }
        else if (strVec[0] == "backup")
        {
            BackupRestaurant *br = new BackupRestaurant();
            br->act(*this);
            actionsLog.push_back(br);
        }
        else if (strVec[0] == "restore")
        {
            RestoreResturant *rr = new RestoreResturant();
            rr->act(*this);
            actionsLog.push_back(rr);
        }
        else if (strVec[0] == "closeall")
        {

            for (int i = 0; i < (int)tables.size(); i++)
            {

                Close *c = new Close(i);
                c->act(*this);
                actionsLog.push_back(c);
            }
            int i = 0;
            while ((int)tables.size() > i)
            {
                delete tables[i];
                i++;
            }
            tables.clear();

            while (actionsLog.size() > 0)
            {
                delete (actionsLog[0]);
                actionsLog.erase(actionsLog.begin());
            }
            actionsLog.clear();
            menu.clear();
        }
    }
}

vector<string> Restaurant::getWordVec(std::string str)
{
    vector<string> inputVec;

    string temp = "";
    int counter = 0;
    for (int i = 0; (unsigned)i < str.length(); i++)
    {
        if ((str[i] != ' ' || str[i] != ','))
        {
            while ((str[i] != ' ' && str[i] != ',') && (unsigned)i < str.length())
            {
                temp += str[i];
                i++;
            }
            inputVec.push_back(temp);
            temp = "";
        }

        if (str[i] == ' ' || str[i] == ',')
            counter++;
    }
    return inputVec;
}

string getFirstWord(string str)
{
    string temp = "";
    for (int i = 0; (unsigned)i < str.length(); i++)
    {
        if (str[i] != ' ')
            temp += str[i];
        else
            i = str.length();
    }
    return temp;
}

string getSecondWord(string str)
{
    string temp = "";
    int counter = 0;
    for (int i = 0; (unsigned)i < str.length(); i++)
    {
        if (counter == 1 && str[i] != ' ')
            temp += str[i];
        if (str[i] == ' ')
            counter++;
    }
    return temp;
}

string getAftertTwo(string str)
{
    string temp = "";
    int counter = 0;
    for (int i = 0; (unsigned)i < str.length(); i++)
    {
        if (counter == 2)
            temp += str[i];
        if (str[i] == ' ')
            counter++;
    }
    return temp;
}

void Restaurant::openTableInRes(std::string str)
{
    std::vector<std::string> inputVec = getWordVec(str);
    int tableId = (int)inputVec[1][0] - 48;
    std::vector<Customer *> custList;
    for (int i = 3; (unsigned)i < inputVec.size(); i += 2)
    {
        if (inputVec[i + 1] == "VEG")
        {

            Customer *veg = new VegetarianCustomer(inputVec[i], (i - 3) / 2);
            custList.push_back(veg);
        }
    }
    BaseAction *o = new OpenTable(tableId, custList);
    actionsLog.push_back(o);
}

Table *Restaurant::getTable(int ind)
{
    return tables.at(ind);
}

std::vector<Dish> &Restaurant::getMenu()
{
    return menu;
}

int Restaurant::getNumOfTables() const
{
    return tables.size();
}

const std::vector<BaseAction *> &Restaurant::getActionsLog() const
{
    return actionsLog;
}

int stringToInt(string str)
{
    int num = 0;
    for (int i = 0; (unsigned)i < str.length(); i++)
    {
        num = num * 10 + (int)str[i] - 48;
    }
    return num;
}

Restaurant &Restaurant::operator=(Restaurant &res)
{
    for (int i = 0; i < (int)res.actionsLog.size(); i++)
    {
        BaseAction *a = res.actionsLog[i]->Clone();
        this->actionsLog.push_back(a);
    }
    this->customerCount = res.customerCount;

    this->numOfTables = res.numOfTables;
    this->open = res.open;
    for (int i = 0; tables.size(); i++)
    {
        if (tables[i] != nullptr)
            delete (tables[i]);
    }

    for (int i = 0; i < (int)res.tables.size(); i++)
    {
        Table *a = new Table(res.tables[i]);
        this->tables.push_back(a);
    }

    return *this;
}
Restaurant::~Restaurant()
{
    int i = 0;
    while ((int)tables.size() > i)
    {
        delete tables[i];

        i++;
    }
    tables.clear();
    while ((int)actionsLog.size() > 0)
    {
        delete (actionsLog[0]);
        actionsLog.erase(actionsLog.begin());
    }
    actionsLog.clear();

    menu.clear();
}
void Restaurant::clear()
{
    int i = 0;
    while ((int)tables.size() > i)
    {
        delete tables[i];
        tables.erase(tables.begin());
    }

    while (actionsLog.size() > 0)
    {
        delete (actionsLog[0]);
        actionsLog.erase(actionsLog.begin());
    }
    actionsLog.clear();

    menu.clear();
}