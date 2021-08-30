#include <iostream>
#include "Dish.h"
#include <string>

Dish::Dish(int d_id, std::string d_name, int d_price, DishType d_type) : id(d_id), name(d_name), price(d_price), type(d_type)
{
}

Dish::Dish(const Dish *d) : id(d->getId()), name(d->getName()), price(d->getPrice()), type(d->getType())
{
}

int Dish::getId() const
{
	return id;
}

std::string Dish::getName() const
{
	return name;
}
int Dish::getPrice() const
{
	return price;
}
DishType Dish::getType() const
{
	return type;
}
void Dish::toString()
{
	if (type == 0)
		std::cout << name << " "
				  << "VEG " << price << "NIS\n";
	if (type == 1)
		std::cout << name << " "
				  << "SPC " << price << "NIS\n";
	if (type == 2)
		std::cout << name << " "
				  << "BVG " << price << "NIS\n";
	if (type == 3)
		std::cout << name << " "
				  << "ALC " << price << "NIS\n";
}
