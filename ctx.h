#pragma once
#include "genericShader.h"
#include "MathHelper.h"
#include "Assert.h"
#include <algorithm>
#include <type_traits>
#include <thread>

namespace Wolverine
{


template<typename VertexShader, 
	typename FragmentShader, //assumes FragmentInput has 'pos' and 'customData' supporting +,- vector operators and scalar *,/ operators
	typename RasterShader, 
	typename OutputBuffer> 
	class CTX
{
public:
	
	void setResolution(unsigned int resX, unsigned int resY)
	{
		m_Res[0] = resX;
		m_Res[1] = resY;
	}

	void setVertexShader(VertexShader &vs)
	{
		m_VertexShader = &vs;
	}

	void setFragmentShader(FragmentShader &fs)
	{
		m_FragmentShader = &fs;
	}

	void setRasterShader(RasterShader &rs)
	{
		m_RasterShader = &rs;
	}

	template<typename VEC2> void findMinMidMaxIdx(VEC2 const (&vertex)[3], unsigned char &outTopLeft, unsigned char &outMiddle, unsigned char &outBottomRight)
	{
		//==============================================================
		//find top left vertex, bottom right vertex and the remaining vertex

		outTopLeft = 0; //top left vertex index

		for (unsigned char idx = 1; idx < 3; ++idx)
		{
			if (vertex[idx][1] <= vertex[outTopLeft][1])
			{
				if (vertex[idx][1] != vertex[outTopLeft][1] || vertex[idx][0] < vertex[outTopLeft][0])
				{
					outTopLeft = idx;
				}
			}
		}

		outMiddle = 1;
		outBottomRight = 2; //bottom right vertex index

		if (outTopLeft == 1)
		{
			outMiddle = 0;
		}
		else if (outTopLeft == 2)
		{
			outBottomRight = 0;
		}

		if (vertex[outBottomRight][1] < vertex[outMiddle][1] ||
			(vertex[outBottomRight][1] == vertex[outMiddle][1] && vertex[outBottomRight][0] < vertex[outMiddle][0]))
		{
			swap(outMiddle, outBottomRight);
		}
	}

	template<typename VEC2> float cross(VEC2 const &edge1, VEC2 const &edge2)
	{
		return edge1[0] * edge2[1] - edge1[1] * edge2[0];
	}


	template<typename VEC2> bool caculateBarycentricWeights(VEC2 const (&vertex)[3], VEC2 const &point, float(&outWeight)[3])
	{
		VEC2 v01 = vertex[1] - vertex[0];
	
		float invDoubleTriangleArea = 1.0f / cross<VEC2>(v01, vertex[2] - vertex[0]);

		outWeight[0] = cross(vertex[2] - vertex[1], point - vertex[2]) * invDoubleTriangleArea;
		outWeight[1] = cross(vertex[0] - vertex[2], point - vertex[0]) * invDoubleTriangleArea;
		outWeight[2] = cross<VEC2>(v01, point - vertex[1]) * invDoubleTriangleArea;
	
		return outWeight[0] >= 0.00001f &&  outWeight[1] >= 0.00001f &&  outWeight[2] >= 0.00001f && outWeight[0] + outWeight[1] + outWeight[2] <= 1.000001f;
	}

	
	template<typename VEC2> bool caculateBarycentricWeights2(float invDoubleTriArea, float doubleTriArea, VEC2 const (&edgesFromTo01_12_20)[3], VEC2 const (&vertex)[3], VEC2 const &point, float(&outWeight)[3])
	{
		outWeight[0] = cross<VEC2>(point - vertex[2], edgesFromTo01_12_20[1]);
		outWeight[1] = cross<VEC2>(point - vertex[0], edgesFromTo01_12_20[2]);
		outWeight[2] = cross<VEC2>(point - vertex[1], edgesFromTo01_12_20[0]);

		if (outWeight[0] >= 0.0f &&  outWeight[1] >= 0.0f &&  outWeight[2] >= 0.0f 
			&& outWeight[0] + outWeight[1] + outWeight[2] <= doubleTriArea * 1.000001f)
		{
			outWeight[0] *= invDoubleTriArea;
			outWeight[1] *= invDoubleTriArea;
			outWeight[2] *= invDoubleTriArea;

			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	template<typename VEC2> bool caculateBarycentricWeights(float invDoubleTriArea, VEC2 const (&edgesFromTo01_12_20)[3], VEC2 const (&vertex)[3], VEC2 const &point, float(&outWeight)[3])
	{
		//In viewport space up and down is switched, thats why we using the opposite orientation
		outWeight[0] = cross<VEC2>(point - vertex[2], edgesFromTo01_12_20[1]) * invDoubleTriArea;
		outWeight[1] = cross<VEC2>(point - vertex[0], edgesFromTo01_12_20[2]) * invDoubleTriArea;
		outWeight[2] = cross<VEC2>(point - vertex[1], edgesFromTo01_12_20[0]) * invDoubleTriArea;

		if (outWeight[0] >= 0.00001f &&  outWeight[1] >= 0.00001f &&  outWeight[2] >= 0.00001f && outWeight[0] + outWeight[1] + outWeight[2] <= 1.000001f)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	

	void calculateRangeToRasterize(Vec2f const vPos[3], unsigned int lineIdx, unsigned char const minMidMaxIdx[3], unsigned int &begin, unsigned int &end)
	{
		if (isEqual(vPos[minMidMaxIdx[2]][1], vPos[minMidMaxIdx[0]][1], 0.001f))
		{
			begin = 0;
			end = 0;
			return;
		}


		float longEdgeOffsetPerLine = (vPos[minMidMaxIdx[2]][0] - vPos[minMidMaxIdx[0]][0]) / (vPos[minMidMaxIdx[2]][1] - vPos[minMidMaxIdx[0]][1]);

		float shortEdgeoffsetPerLine;

		unsigned char endVertIdx = 0;
		if (lineIdx <= vPos[minMidMaxIdx[1]][1]) //upper part of the triangle
		{
			if (isEqual(vPos[minMidMaxIdx[1]][1], vPos[minMidMaxIdx[0]][1], 0.001f))
			{
				begin = vPos[minMidMaxIdx[0]][0];
				end = vPos[minMidMaxIdx[1]][0];

				return;
			}
			else
			{
				endVertIdx = 0; 
				shortEdgeoffsetPerLine = (vPos[minMidMaxIdx[1]][0] - vPos[minMidMaxIdx[0]][0]) / (vPos[minMidMaxIdx[1]][1] - vPos[minMidMaxIdx[0]][1]);
			}
			
		}
		else //bottom part of the triangle
		{
			endVertIdx = 1;
			shortEdgeoffsetPerLine = (vPos[minMidMaxIdx[2]][0] - vPos[minMidMaxIdx[1]][0]) / (vPos[minMidMaxIdx[2]][1] - vPos[minMidMaxIdx[1]][1]);
		}

		begin = max(0.0f,  vPos[minMidMaxIdx[0]][0] + (longEdgeOffsetPerLine * lineIdx));
		end = max(0.0f,vPos[minMidMaxIdx[endVertIdx]][0] + (shortEdgeoffsetPerLine * lineIdx));

		if (begin > end)
		{
			swap(begin, end);
		}

	}

	
	float calculateWeight(Vec2f const &v0, Vec2f const &v1, Vec2f const &point)
	{
		return cross(point - v1, v0 - v1);
	}

	
	float orient2d(const Vec2f& a, const Vec2f& b, const Vec2f& c)
{
    return (b[0]-a[0])*(c[1] -a[1]) - (b[1] -a[1])*(c[0] -a[0]);
}
	
	template <typename VertexData> void rasterizeBarycentricThread(int minX, int maxX, int minY, int maxY, VertexData const &vertex0, VertexData const &vertex1, VertexData const &vertex2, int startX, int startY, int offsetX, int offsetY)
	{
		Vec2f const edgesFromTo01_12_20[3]=
		{
			Vec2f(vertex1.pos[0] - vertex0.pos[0], vertex1.pos[1] - vertex0.pos[1]),
			Vec2f(vertex2.pos[0] - vertex1.pos[0], vertex2.pos[1] - vertex1.pos[1]),
			Vec2f(vertex0.pos[0] - vertex2.pos[0], vertex0.pos[1] - vertex2.pos[1]) 
		};

		//in viewport space up and down is switched, thats why '-'
		float const doubleTriangleArea = -cross(edgesFromTo01_12_20[0], edgesFromTo01_12_20[1]);
		float const invDoubleTriangleArea = 1.0f / doubleTriangleArea;


		float weight[3];

		float invZ[3] = { 1.0f / vertex0.pos[2], 1.0f / vertex1.pos[2], 1.0f / vertex2.pos[2] };

		Vec2f p(minX + startX, minY + startY);

		Vec2f vPos[3] = {
			Vec2f(vertex0.pos[0], vertex0.pos[1]),
			Vec2f(vertex1.pos[0], vertex1.pos[1]),
			Vec2f(vertex2.pos[0], vertex2.pos[1])
		};

		auto interpolatedFragment = vertex0;

		//float rowWeights[3];
		//caculateBarycentricWeights(vPos, p, rowWeights);

		float row_w0 = -calculateWeight(vPos[1], vPos[2], p) * invDoubleTriangleArea;
		float row_w1 = -calculateWeight(vPos[2], vPos[0], p) * invDoubleTriangleArea;
		float row_w2 = -calculateWeight(vPos[0], vPos[1], p) * invDoubleTriangleArea;

		float A01 = -(vPos[0][1] - vPos[1][1]) * invDoubleTriangleArea;
		float B01 = -(vPos[1][0] - vPos[0][0]) * invDoubleTriangleArea;

		float A12 = -(vPos[1][1] - vPos[2][1]) * invDoubleTriangleArea;
		float B12 = -(vPos[2][0] - vPos[1][0]) * invDoubleTriangleArea;

		float A20 = -(vPos[2][1] - vPos[0][1]) * invDoubleTriangleArea;
		float B20 = -(vPos[0][0] - vPos[2][0]) * invDoubleTriangleArea;

		for (int y = minY+startY; y <= maxY; y+=offsetY )
		{
			weight[0] = row_w0;
			weight[1] = row_w1;
			weight[2] = row_w2;

			p[1] = y;

			for (int x = minX+startX; x <= maxX; x+=offsetX )
			{
				p[0] = x;

				
				if (weight[0] >= 0 && weight[1] >= 0 && weight[2] >= 0)
				//if (caculateBarycentricWeights2(invDoubleTriangleArea, doubleTriangleArea, edgesFromTo01_12_20, vPos, p, weight))
				//if (caculateBarycentricWeights(invDoubleTriangleArea, edgesFromTo01_12_20, vPos, p, weight))
				//if (caculateBarycentricWeights(vPos, p, weight))
				{
					//interpolate values

					float z = 1.0f / (weight[0] * invZ[0] + weight[1] * invZ[1] + weight[2] * invZ[2]);

					interpolatedFragment.pos = ((vertex0.pos * (weight[0] * invZ[0])) + (vertex1.pos * (weight[1] * invZ[1])) + (vertex2.pos * (weight[2] * invZ[2]))) * z;
					interpolatedFragment.customData = ((vertex0.customData * (weight[0] * invZ[0])) + (vertex1.customData * (weight[1] * invZ[1])) + (vertex2.customData * (weight[2] * invZ[2]))) * z;

					//m_RasterShader->process(x, y, vPos[0]);
					m_RasterShader->process(x, y, m_FragmentShader->process(interpolatedFragment));
				}
				
				weight[0] += A12;
				weight[1] += A20;
				weight[2] += A01;
			}

			row_w0 += B12;
			row_w1 += B20;
			row_w2 += B01;
		}

	}

	template <typename VertexData> void rasterizeBarycentric(VertexData const &vertex0, VertexData const &vertex1, VertexData const &vertex2)
	{
		int minX =  max(0, (int)min({ vertex0.pos[0], vertex1.pos[0], vertex2.pos[0] }));
		int maxX = min((int) m_Res[0]-1, (int)max({ vertex0.pos[0], vertex1.pos[0], vertex2.pos[0] }));

		int minY = max(0,(int)min({ vertex0.pos[1], vertex1.pos[1], vertex2.pos[1] }));
		int maxY = min((int) m_Res[1]-1, (int)max({ vertex0.pos[1], vertex1.pos[1], vertex2.pos[1] }));

		
		auto lambda1 = [=] { rasterizeBarycentricThread(minX, maxX, minY, maxY, vertex0, vertex1, vertex2, 0, 0,1, 1); };
		//auto lambda2 = [=] { rasterizeBarycentricThread(minX, maxX, minY, maxY, vertex0, vertex1, vertex2, 1, 0, 2, 1); };

		//auto lambdaTest = [=] { rasterizeBarycentricThread(minX, maxX, minY, maxY, vertex0, vertex1, vertex2, 0, 0, 2, 2); };

		//auto lambdaTest = [=] { int i = 0; };

		//std::thread thread1(lambdaTest);
		//std::thread thread2(lambdaTest);
		
		//thread1.join();
		//thread2.join();

		lambda1();
		//lambda2();
		
		//lambdaTest();
	}

	template<typename VertexBuffer, typename IndexBuffer> void renderRange(VertexBuffer const &vertexBuffer, IndexBuffer const &indexBuffer, size_t fromIdx, size_t toIdx)
	{
		for (size_t idx = fromIdx; idx < toIdx; idx += 3)
		{
			//TODO add some cache, this process each vertex multiple times!!!
			auto fragmentInput0 = m_VertexShader->process(vertexBuffer[indexBuffer[idx]]);
			auto fragmentInput1 = m_VertexShader->process(vertexBuffer[indexBuffer[idx + 1]]);
			auto fragmentInput2 = m_VertexShader->process(vertexBuffer[indexBuffer[idx + 2]]);

			//camera is at (0,0,0) looking in (0,0,-1)
			if (fragmentInput0.pos[2] < 0 || fragmentInput1.pos[2] < 0 || fragmentInput2.pos[2] < 0)
			{
				//TODO split polygon if not all 3 vertiices are behind :/

				//behind camera
				continue;
			}

			//NOTE: We changed mapping -> vector.Y are flipped now, thats why cross product is switched as well here
			if (gmtl::makeCross(ConvertToVec3(fragmentInput1.pos - fragmentInput0.pos), ConvertToVec3(fragmentInput2.pos - fragmentInput1.pos))[2] >= 0.0f)
			{
				//polygon is not facing camera
				continue;
			}

			rasterizeBarycentric(fragmentInput0, fragmentInput1, fragmentInput2);
		}
	}

	template<typename VertexBuffer, typename IndexBuffer> void render( VertexBuffer const &vertexBuffer, IndexBuffer const &indexBuffer)
	{
		ASSERT(m_Res[0] > 0 && m_Res[1] > 0, "Invalid resolution");
		ASSERT(indexBuffer.size() % 3 == 0, "Index buffer must consist of triplets");
		ASSERT(nullptr != m_VertexShader, "Vertex shader cant be null");
		ASSERT(nullptr != m_FragmentShader, "Fragment shader cant be null");
		ASSERT(nullptr != m_RasterShader, "Raster shader cant be null");

		//TODO CLIPPING ... vertices needs to be in projection space to do that

	
		
		auto lambda1 = [&, this] { renderRange(vertexBuffer, indexBuffer, 0, indexBuffer.size()); };
		//auto lambda2 = [&, this] { renderRange(vertexBuffer, indexBuffer, 1068, indexBuffer.size()); };
		
		//std::thread thread1(lambda1);
		//std::thread thread2(lambda2);

		//thread1.join();
		//thread2.join();
		
		//lambda1();
		lambda1();

		//renderRange(vertexBuffer, indexBuffer, 0, indexBuffer.size());
	}

private:

	unsigned int m_Res[2] = { 0,0 };
	VertexShader *m_VertexShader;
	FragmentShader *m_FragmentShader;
	RasterShader *m_RasterShader;
};


}