#pragma once

#include "../Base/ArithmeticalPack.h"
#include "Pack4b.h"

#include <xmmintrin.h>

class Pack4f;

template <>
struct PackTraits<Pack4f>
{
	typedef float valueType;
	typedef Pack4b logicalPack;
};

/*
Process arithmetical operations on 4 floats using SSE instructions
*/
class Pack4f : public ArithmeticPack<Pack4f>
{
public:
	typedef PackTraits<Pack4f>::valueType valueType;
	typedef PackTraits<Pack4f>::logicalPack logicalPack;

	Pack4f(valueType f0, valueType f1, valueType f2, valueType f3) : m_Val(_mm_setr_ps(f0, f1, f2, f3))
	{
	}

	Pack4f(valueType f) : m_Val(_mm_set1_ps(f))
	{
	}

	Pack4f(__m128 const &val) :m_Val(val)
	{
	}

	operator __m128() const
	{
		return m_Val;
	}

	Pack4f operator+(Pack4f const &op)
	{
		return _mm_add_ps(*this, op);
	}

	Pack4f operator-(Pack4f const &op)
	{
		return _mm_sub_ps(*this, op);
	}

	Pack4f operator*(Pack4f const &op)
	{
		return _mm_mul_ps(*this, op);
	}

	Pack4f operator/(Pack4f const &op)
	{
		return _mm_div_ps(*this, op);
	}

	logicalPack operator>=(Pack4f const &op)
	{
		return _mm_cmpge_ps(*this, op);
	}

private:
	__m128 m_Val;
};
