#pragma once
#include "../Base/LogicalPack.h"
#include <xmmintrin.h>

/*
Process logical operations on 4 booleans using SSE instructions
*/
class Pack4b : public LogicalPack<Pack4b>
{
public:
	Pack4b(__m128 const &val) : m_Val(val)
	{
	}

	operator __m128() const
	{
		return m_Val;
	}

	Pack4b operator&&(Pack4b const &op)
	{
		return _mm_and_ps(m_Val, op.m_Val);
	}

private:
	__m128 m_Val;
};