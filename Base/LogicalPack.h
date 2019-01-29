#pragma once

/*
Base class for logical operations on vectors
You should inherit from this class whenever you creating new class using basic logical operations such as &&, ||
*/
template< typename FinalType>
class LogicalPack
{
public:

	LogicalPack& operator()()
	{
		return *static_cast<FinalType*>(this);
	}

	const FinalType& operator()() const
	{
		return *static_cast<const FinalType*>(this);
	}

	FinalType& operator&=(FinalType const &op)
	{
		*this = *this && op;
	}

	FinalType& operator|=(FinalType const &op)
	{
		*this = *this || op;
	}

	const int* GetDataPtr() const
	{
		return reinterpret_cast<const int *>(this);
	}
};