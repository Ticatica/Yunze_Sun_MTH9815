/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

  // ctor for a position
  Position(const T &_product);

  // Get the product
  const T& GetProduct() const;

  // Get the position quantity
  long GetPosition(string &book);

  // Get the aggregate position
  long GetAggregatePosition();

  // New created - Change the position
  void AddPosition(string id, long _position);

private:
  T product;
  map<string,long> positions; // book -> pos for the product

};

/**
 * Position Service to manage positions across multiple books and secruties.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string,Position <T> >
{

public:

  // Add a trade to the service
  virtual void AddTrade(const Trade<T> &trade) = 0;

};

template<typename T>
Position<T>::Position(const T &_product) :
  product(_product)
{
}

template<typename T>
const T& Position<T>::GetProduct() const
{
  return product;
}

template<typename T>
long Position<T>::GetPosition(string &book)
{
  return positions[book];
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
  // (No-op implementation - should be filled out for implementations) finished
    long sum = 0;
    for (auto& pos : positions) {
        sum += pos.second;
    }
    return sum;
}

// 
template<typename T>
void Position<T>::AddPosition(string book, long position) {
    if (positions.find(book) != positions.end()) {
        // already in the map, change the position
        positions[book] += position;
    }
    else {
        // not in the map, create a new element
        positions.insert(pair<string, long>(book, position));
    }
}

#endif
