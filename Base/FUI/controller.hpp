#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <functional>
#include <vector>
#include <utility>
#include <any>

/*
 * This templated class takes a reference of classes in any variety. It uses
 * variadic templates which has been hard to understand. However, it just means
 * I can accept any number of classes as an argument.
 *
 * From the constructor, it creates a tuple of classes by reference. From there,
 * it waits until the funtion setValues or getValues is called. It then does
 * crazy lambda magic, unpacks the variadic template, and applies the function
 * for each class.
 *
 * TODO: Change x to Class
 */
template<typename...Classes>
class Controller
{
  public:
    Controller(Classes&...objects) : objects(objects...){}
    Controller(std::tuple<Classes&...> tup) : objects(tup){}

    template<typename T>
    void setValues(T value)
    {
      std::apply([&](auto&...x) { (x.updateValue(value),...);}, objects);
    }

    void getValues(std::vector<std::any> &values) const
    {
      std::apply([&](auto&...x) { (values.push_back(x.get()),...);}, objects);
    }

    std::tuple<Classes&...> getObjects() const
    {
      return objects;
    }

    constexpr std::size_t getSize() const
    {
      return std::tuple_size<decltype(objects)>::value;
    }

  private:
    std::tuple<Classes&...> objects;
};

template<typename...ClassesL, typename...ClassesR>
auto operator+(const Controller<ClassesL...>& lhs, const Controller<ClassesR...>& rhs)
{
  return Controller(std::tuple_cat(lhs.getObjects(),rhs.getObjects()));
}

template<typename...ClassesL, typename ClassesR>
auto operator+(const Controller<ClassesL...> &lhs, ClassesR rhs)
{
  Controller<ClassesR> makeController(rhs);
  return Controller(std::tuple_cat(lhs.getObjects(),makeController.getObjects()));
}

template<typename ClassesL, typename...ClassesR>
auto operator+(ClassesL lhs, const Controller<ClassesR...> &rhs)
{
  Controller<ClassesL> makeController(lhs);
  return Controller(std::tuple_cat(makeController.getObjects(),rhs.getObjects()));
}

#endif // CONTROLLER_HPP
