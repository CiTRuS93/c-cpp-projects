
#include <vector>
#include <string>
#include "Dish.h"
#include "Customer.h"
///// ================== Customer ================== /////
Customer::Customer(std::string c_name, int c_id) : name(c_name), id(c_id){};

Customer::~Customer()
{
}

std::string Customer::getName() const
{
    return name;
}
int Customer::getId() const
{
    return id;
}

///// ================== VegetarianCustomer ================== /////
VegetarianCustomer::VegetarianCustomer(std::string name, int id) : Customer(name, id)
{
}
VegetarianCustomer::~VegetarianCustomer()
{
}
VegetarianCustomer::VegetarianCustomer(const VegetarianCustomer &c) : Customer(c.getName(), c.getId())
{
}
Customer *VegetarianCustomer::Clone()
{
    return new VegetarianCustomer(*this);
}
std::vector<int> VegetarianCustomer::order(const std::vector<Dish> &menu)
{
    // orders the vegetarian dish with the smallest id in the menu, and the most expensive beverage(Non - alcoholic)
    //find the lowest id VEG
    ordered = true;
    int size = menu.size();
    int smallestVegId = -1;

    for (int i = 0; i < size; i++)
    {
        if (menu[i].getType() == 0)
        { // if type of veg
            if (smallestVegId == -1)
                smallestVegId = menu[i].getId();
            else if (menu[i].getId() < smallestVegId)
            {
                smallestVegId = menu[i].getId();
            }
        }
    }
    //find the pricest BVG
    int pricestBVGId = -1;
    int price = -1;
    for (int i = 0; i < size; i++)
    {
        if (menu[i].getType() == 2)
        { // if type of bvg
            if (pricestBVGId == -1)
            { //start value
                pricestBVGId = menu[i].getId();
                price = menu[i].getPrice();
            }
            else if ((price < menu[i].getPrice()) || (price == menu[i].getPrice() && pricestBVGId > menu[i].getId()))
            { //swap if price is lower or equal and id lower
                pricestBVGId = menu[i].getId();
                price = menu[i].getPrice();
            }
        }
    }
    std::vector<int> dishesId;
    dishesId.push_back(smallestVegId);
    dishesId.push_back(pricestBVGId);

    return dishesId;
}
std::string VegetarianCustomer::toString() const
{
    return "" + std::to_string(this->getId()) + " " + this->getName();
}
std::string VegetarianCustomer::getType()
{
    return "veg";
}

///// ================== CheapCustomer ================== /////
CheapCustomer::CheapCustomer(std::string name, int id) : Customer(name, id) {}
CheapCustomer::~CheapCustomer()
{
}
CheapCustomer::CheapCustomer(const CheapCustomer &c) : Customer(c.getName(), c.getId())
{
}
Customer *CheapCustomer::Clone()
{
    return new CheapCustomer(*this);
}
std::vector<int> CheapCustomer::CheapCustomer::order(const std::vector<Dish> &menu)
{
    //orders the cheapest dish in the menu.
    //This customer orders only once
    std::vector<int> orders;
    if (ordered == false)
    {
        int cheapest = menu[0].getPrice();
        int cheapestId = menu[0].getId();
        for (int i = 0; (unsigned)i < menu.size(); i++)
        {
            if (menu[i].getPrice() < cheapest)
            {
                cheapest = menu[i].getPrice();
                cheapestId = menu[i].getId();
            }
        }
        orders.push_back(cheapestId);
        ordered = true;
    }
    return orders;
}
std::string CheapCustomer::toString() const
{
    return "" + std::to_string(this->getId()) + " " + this->getName();
}
std::string CheapCustomer::getType()
{
    return "chp";
}

///// ================== SpicyCustomer ================== /////
SpicyCustomer::SpicyCustomer(std::string name, int id) : Customer(name, id) {}
SpicyCustomer::~SpicyCustomer()
{
}
SpicyCustomer::SpicyCustomer(const SpicyCustomer &c) : Customer(c.getName(), c.getId())
{
}
Customer *SpicyCustomer::Clone()
{
    return new SpicyCustomer(*this);
}
std::vector<int> SpicyCustomer::order(const std::vector<Dish> &menu)
{ // orders the most expensive spicy dish in the menu.
    //For further orders, he picks the cheapest non - alcoholic beverage in the menu.
    //The order might be equal to previous orders
    std::vector<int> orders;
    if (firstOrder == false)
    {
        int pricest = -1;
        int pricestId = -1;
        for (int i = 0; (unsigned)i < menu.size(); i++)
        {
            if (pricestId == -1 && menu[i].getType() == 1)
            {
                pricest = menu[i].getPrice();
                pricestId = menu[i].getId();
            }
            else if (menu[i].getPrice() >= pricest && menu[i].getType() == 1)
            {
                if ((menu[i].getPrice() == pricest && menu[i].getId() < pricestId) || menu[i].getPrice() > pricest)
                {
                    pricest = menu[i].getPrice();
                    pricestId = menu[i].getId();
                }
            }
        }
        firstOrder = true;
        orders.push_back(pricestId);
    }
    else
    {
        int cheapest = -1;
        int cheapestId = -1;
        for (int i = 0; (unsigned)i < menu.size(); i++)
        {
            if (cheapestId == -1 && menu[i].getType() == 2)
            {
                cheapest = menu[i].getPrice();
                cheapestId = menu[i].getId();
            }
            else if (menu[i].getPrice() <= cheapest && menu[i].getType() == 2)
            {
                if ((menu[i].getPrice() == cheapest && menu[i].getId() < cheapestId) || menu[i].getPrice() < cheapest)
                {
                    cheapest = menu[i].getPrice();
                    cheapestId = menu[i].getId();
                }
            }
        }
        orders.push_back(cheapestId);
    }
    return orders;
}

std::string SpicyCustomer::toString() const
{
    return "" + std::to_string(this->getId()) + " " + this->getName();
}

std::string SpicyCustomer::getType()
{
    return "spc";
}

///// ================== AlchoholicCustomer ================== /////
AlchoholicCustomer::AlchoholicCustomer(std::string name, int id) : Customer(name, id)
{
    idNumber = -1;
}
AlchoholicCustomer::~AlchoholicCustomer()
{
}
AlchoholicCustomer::AlchoholicCustomer(const AlchoholicCustomer &c) : Customer(c.getName(), c.getId())
{
}
Customer *AlchoholicCustomer::Clone()
{
    return new AlchoholicCustomer(*this);
}
std::vector<int> AlchoholicCustomer::order(const std::vector<Dish> &menu)
{
    std::vector<int> dishes;
    int cheapestId = -1;
    int cheapest = -1;
    int inOrdered = 0;
    for (int i = 0; i < (int)menu.size(); i++)
    {
        if (menu[i].getType() == 3 && cheapest == -1)
        {
            for (int j = 0; j < (int)idOrdered.size(); j++)
            {
                if (idOrdered[j] == i)
                {
                    inOrdered = 1;
                }
            }
            if (inOrdered != 1)
            {
                cheapest = menu[i].getPrice();
                cheapestId = menu[i].getId();
            }
            inOrdered = 0;
        }
        if (menu[i].getPrice() < cheapest && menu[i].getType() == 3)
        {
            for (int j = 0; j < (int)idOrdered.size(); j++)
            {
                if (idOrdered[j] == i)
                {
                    inOrdered = 1;
                }
            }
            if (inOrdered != 1)
            {
                cheapest = menu[i].getPrice();
                cheapestId = menu[i].getId();
            }
            inOrdered = 0;
        }
    }
    idOrdered.push_back(cheapestId);
    dishes.push_back(cheapestId);
    return dishes;
}

std::string AlchoholicCustomer::toString() const
{
    return "" + std::to_string(this->getId()) + " " + this->getName();
}
std::string AlchoholicCustomer::getType()
{
    return "alc";
}