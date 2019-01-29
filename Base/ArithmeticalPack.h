#pragma once


/*
Trait class that connects ArithmetickPack with corresponding LogicalPack,
You need to define valueType for each class derived from ArithmeticPack and corresponding logicalPack
*/
template <typename T>
struct PackTraits;
/*
typedef YOUR_VALUE_TYPE valueType;
typedef YOUR_LOGICAL_PACK logicalPack;
*/


/*
Base class for arithmetical operations on vectors
You should inherit from this class whenever you creating new class using basic arithmetic operations such as +,-,*,/
*/
template<typename FinalType>
class ArithmeticPack
{
public:

	// downcast operators so we can call methods in the inheriting classes
	FinalType& operator()() { return *static_cast<FinalType*>(this); }
	const FinalType& operator()() const { return *static_cast<const FinalType*>(this); }

	FinalType& operator+=(FinalType const &op)
	{
		(*this)() = (*this)() + op;
		return (*this)();
	}

	FinalType& operator-=(FinalType const &op)
	{
		*this = *this - op;
		return *this;
	}

	FinalType& operator*=(FinalType const &op)
	{
		*this = *this * op;
		return *this;
	}

	FinalType& operator/=(FinalType const &op)
	{
		*this = *this / op;
		return *this;
	}

	const typename PackTraits<FinalType>::valueType* GetDataPtr() const
	{
		return reinterpret_cast<const typename PackTraits<FinalType>::valueType* >(this);
	}

};