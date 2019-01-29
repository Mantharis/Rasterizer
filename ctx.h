#pragma once
#include "genericShader.h"
#include "MathHelper.h"
#include "Assert.h"
#include <algorithm>
#include <type_traits>
#include <thread>
#include <array>

#include "SIMD\SSE\Pack4b.h"
#include "SIMD\SSE\Pack4f.h"

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

	float calculateWeight(Vec2f const &v0, Vec2f const &v1, Vec2f const &point)
	{
		return cross(point - v1, v0 - v1);
	}

	template <typename VertexData> void rasterizeBarycentricThread(int minX, int maxX, int minY, int maxY, VertexData const &vertex0, VertexData const &vertex1, VertexData const &vertex2)
	{
		Vec2f e01 = Vec2f(vertex1.pos[0] - vertex0.pos[0], vertex1.pos[1] - vertex0.pos[1]);
		Vec2f e12 = Vec2f(vertex2.pos[0] - vertex1.pos[0], vertex2.pos[1] - vertex1.pos[1]);

		//in viewport space up and down is switched, thats why '-'
		float const doubleTriangleArea = -cross(e01, e12);
		float const invDoubleTriangleArea = 1.0f / doubleTriangleArea;

		Vec3f invZ(1.0f / vertex0.pos[3], 1.0f / vertex1.pos[3], 1.0f / vertex2.pos[3]);

		Vec2f vPos[3] = {
			Vec2f(vertex0.pos[0], vertex0.pos[1]),
			Vec2f(vertex1.pos[0], vertex1.pos[1]),
			Vec2f(vertex2.pos[0], vertex2.pos[1])
		};

		Vec2f firstFragment(minX, minY);

		float row_w0 = -calculateWeight(vPos[1], vPos[2], firstFragment) * invDoubleTriangleArea;
		float row_w1 = -calculateWeight(vPos[2], vPos[0], firstFragment) * invDoubleTriangleArea;
		float row_w2 = -calculateWeight(vPos[0], vPos[1], firstFragment) * invDoubleTriangleArea;

		float A01 = -(vPos[0][1] - vPos[1][1]) * invDoubleTriangleArea;
		float B01 = -(vPos[1][0] - vPos[0][0]) * invDoubleTriangleArea;

		float A12 = -(vPos[1][1] - vPos[2][1]) * invDoubleTriangleArea;
		float B12 = -(vPos[2][0] - vPos[1][0]) * invDoubleTriangleArea;

		float A20 = -(vPos[2][1] - vPos[0][1]) * invDoubleTriangleArea;
		float B20 = -(vPos[0][0] - vPos[2][0]) * invDoubleTriangleArea;

		Pack4f pack_A01_mult4(A01 * 4.0f);
		Pack4f pack_A12_mult4(A12 * 4.0f);
		Pack4f pack_A20_mult4(A20 * 4.0f);

		//@float weight[3];

		Pack4f packZero(0.0f);
		Pack4f packOne(1.0f);
		Pack4f packWeight[3] = { 0.0f, 0.0f, 0.0f };



		for (int y = minY; y <= maxY; ++y)
		{
			packWeight[0] = Pack4f(row_w0, row_w0 + A12, row_w0 + A12 * 2, row_w0 + A12 * 3);
			packWeight[1] = Pack4f(row_w1, row_w1 + A20, row_w1 + A20 * 2, row_w1 + A20 * 3);
			packWeight[2] = Pack4f(row_w2, row_w2 + A01, row_w2 + A01 * 2, row_w2 + A01 * 3);

			//@weight[0] = row_w0;
			//@weight[1] = row_w1;
			//@weight[2] = row_w2;

			for (int x = minX; x <= maxX; x += 4)
			{

				//@if (weight[0] >= 0 && weight[1] >= 0 && weight[2] >= 0)

				//TODO .GetDataPtr()[0] 
				if ((packWeight[0] >= packZero).GetDataPtr()[0] && (packWeight[1] >= packZero).GetDataPtr()[0] && (packWeight[2] >= packZero).GetDataPtr()[0])
				{
					//TODO I can calculate this incrementally
					//@Vec3f weightDivZ (weight[0] * invZ[0], weight[1] * invZ[1], weight[2] * invZ[2]);

					Pack4f packWeightDivZ[3] = { Pack4f(0.0f), Pack4f(0.0f), Pack4f(0.0f) }; //[0]=x, [1]=y, [2]=z
					packWeightDivZ[0] = Pack4f(packWeight[0] * invZ[0]);
					packWeightDivZ[1] = Pack4f(packWeight[1] * invZ[1]);
					packWeightDivZ[2] = Pack4f(packWeight[2] * invZ[2]);

					//@float z = 1.0f / (weightDivZ[0] + weightDivZ[1] + weightDivZ[2]);

					Pack4f packZ = packOne / (packWeightDivZ[0] + packWeightDivZ[1] + packWeightDivZ[2]);

					//@FragmentData fragment;

					Pack4f packFragmentData[3] = { 0.0f, 0.0f, 0.0f };

					//@float fragmentDepth = ((vertex0.pos[2] * weightDivZ[0]) + (vertex1.pos[2] * weightDivZ[1]) + (vertex2.pos[2] * weightDivZ[2])) * z;

					//Pack4f packFragmentDepth = ((packWeightDivZ[0] * vertex0.pos[3]) + (packWeightDivZ[1] * vertex1.pos[3]) + (packWeightDivZ[2] * vertex2.pos[3])) * packZ;
					Pack4f packFragmentDepth = ((packWeightDivZ[0] * vertex0.pos[2]) + (packWeightDivZ[1] * vertex1.pos[2]) + (packWeightDivZ[2] * vertex2.pos[2])) * packZ;



					/*
					//early depth test
					if (m_RasterShader->getDepth(x, y) < fragmentDepth)
					{
					weight[0] += A12;
					weight[1] += A20;
					weight[2] += A01;

					continue;
					}
					*/

					//TODO I can do early depth test here and dont interpolate fragment at all if it will be rejected by depth test anyway
					//@Vec2f fragmentTexCoord = ((vertex0.customData * weightDivZ[0]) + (vertex1.customData * weightDivZ[1]) + (vertex2.customData * weightDivZ[2])) * z;

					Pack4f packFragmentTexCoord[2] = { 0.0f, 0.0f }; // [0]==x, [1]==y
					packFragmentTexCoord[0] = ((packWeightDivZ[0] * vertex0.customData[0]) + (packWeightDivZ[1] * vertex1.customData[0]) + (packWeightDivZ[2] * vertex2.customData[0])) * packZ;
					packFragmentTexCoord[1] = ((packWeightDivZ[0] * vertex0.customData[1]) + (packWeightDivZ[1] * vertex1.customData[1]) + (packWeightDivZ[2] * vertex2.customData[1])) * packZ;


					packFragmentData[0] = packFragmentTexCoord[0];
					packFragmentData[1] = packFragmentTexCoord[1];
					packFragmentData[2] = packFragmentDepth;


					//TODO: copying from one place to another
					//@fragment[0] = fragmentTexCoord[0];
					//@fragment[1] = fragmentTexCoord[1];
					//@fragment[2] = fragmentDepth;

					/*
					for (int i = 0; i < 4; ++i)
					{
						Vec3b color;
						float depth;

						Vec3f tmpFragment;
						tmpFragment[0] = packFragmentData[0].GetDataPtr()[i];
						tmpFragment[1] = packFragmentData[1].GetDataPtr()[i];
						tmpFragment[2] = packFragmentData[2].GetDataPtr()[i];

						m_FragmentShader->process(tmpFragment, color, depth);
						m_RasterShader->process(x + i, y, color, depth);
					}
					*/

					Vec3b color[4];
					float depth[4];

					m_FragmentShader->process(packFragmentData[0], packFragmentData[1], packFragmentData[2], color, depth);

					for (int i = 0; i < 4; ++i)
					{
						m_RasterShader->process(x + i, y, color[i], depth[i]);
					}

				}

				packWeight[0] += pack_A12_mult4;
				packWeight[1] += pack_A20_mult4;
				packWeight[2] += pack_A01_mult4;

				//@weight[0] += A12;
				//@weight[1] += A20;
				//@weight[2] += A01;
			}

			row_w0 += B12;
			row_w1 += B20;
			row_w2 += B01;
		}

	}

#ifdef NO_SIMD
	template <typename VertexData> void rasterizeBarycentricThread(int minX, int maxX, int minY, int maxY, VertexData const &vertex0, VertexData const &vertex1, VertexData const &vertex2)
	{
		Vec2f e01 = Vec2f(vertex1.pos[0] - vertex0.pos[0], vertex1.pos[1] - vertex0.pos[1]);
		Vec2f e12 = Vec2f(vertex2.pos[0] - vertex1.pos[0], vertex2.pos[1] - vertex1.pos[1]);

		//in viewport space up and down is switched, thats why '-'
		float const doubleTriangleArea = -cross(e01, e12);
		float const invDoubleTriangleArea = 1.0f / doubleTriangleArea;

		Vec3f invZ(1.0f / vertex0.pos[2], 1.0f / vertex1.pos[2], 1.0f / vertex2.pos[2]);

		Vec2f vPos[3] = {
			Vec2f(vertex0.pos[0], vertex0.pos[1]),
			Vec2f(vertex1.pos[0], vertex1.pos[1]),
			Vec2f(vertex2.pos[0], vertex2.pos[1])
		};

		Vec2f firstFragment(minX, minY);

		float row_w0 = -calculateWeight(vPos[1], vPos[2], firstFragment) * invDoubleTriangleArea;
		float row_w1 = -calculateWeight(vPos[2], vPos[0], firstFragment) * invDoubleTriangleArea;
		float row_w2 = -calculateWeight(vPos[0], vPos[1], firstFragment) * invDoubleTriangleArea;

		float A01 = -(vPos[0][1] - vPos[1][1]) * invDoubleTriangleArea;
		float B01 = -(vPos[1][0] - vPos[0][0]) * invDoubleTriangleArea;

		float A12 = -(vPos[1][1] - vPos[2][1]) * invDoubleTriangleArea;
		float B12 = -(vPos[2][0] - vPos[1][0]) * invDoubleTriangleArea;

		float A20 = -(vPos[2][1] - vPos[0][1]) * invDoubleTriangleArea;
		float B20 = -(vPos[0][0] - vPos[2][0]) * invDoubleTriangleArea;

		Vec3f weight;

		for (int y = minY; y <= maxY; ++y )
		{
			weight[0] = row_w0;
			weight[1] = row_w1;
			weight[2] = row_w2;

			for (int x = minX; x <= maxX; ++x )
			{
				
				if (weight[0] >= 0 && weight[1] >= 0 && weight[2] >= 0)
				{
					//TODO I can calculate this incrementally
					Vec3f weightDivZ (weight[0] * invZ[0], weight[1] * invZ[1], weight[2] * invZ[2]);

					float z = 1.0f / (weightDivZ[0] + weightDivZ[1] + weightDivZ[2]);
					
					FragmentData fragment;
					
					float fragmentDepth = ((vertex0.pos[2] * weightDivZ[0]) + (vertex1.pos[2] * weightDivZ[1]) + (vertex2.pos[2] * weightDivZ[2])) * z;

					/*
					//early depth test
					if (m_RasterShader->getDepth(x, y) < fragmentDepth)
					{
						weight[0] += A12;
						weight[1] += A20;
						weight[2] += A01;

						continue;
					}
					*/

					//TODO I can do early depth test here and dont interpolate fragment at all if it will be rejected by depth test anyway
					Vec2f fragmentTexCoord = ((vertex0.customData * weightDivZ[0]) + (vertex1.customData * weightDivZ[1]) + (vertex2.customData * weightDivZ[2])) * z;
					
					//TODO: copying from one place to another
					fragment[0] = fragmentTexCoord[0];
					fragment[1] = fragmentTexCoord[1];
					fragment[2] = fragmentDepth;
					
					Vec3b color;
					float depth;

					m_FragmentShader->process(fragment, color, depth);
					m_RasterShader->process(x, y, color, depth);
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
#endif

	template <typename VertexData> void rasterizeBarycentric(VertexData const &vertex0, VertexData const &vertex1, VertexData const &vertex2)
	{
		int minX =  max(0, (int)min({ vertex0.pos[0], vertex1.pos[0], vertex2.pos[0] }));
		int maxX = min((int) m_Res[0]-1, (int)max({ vertex0.pos[0], vertex1.pos[0], vertex2.pos[0] }));

		int minY = max(0,(int)min({ vertex0.pos[1], vertex1.pos[1], vertex2.pos[1] }));
		int maxY = min((int) m_Res[1]-1, (int)max({ vertex0.pos[1], vertex1.pos[1], vertex2.pos[1] }));

		rasterizeBarycentricThread(minX, maxX, minY, maxY, vertex0, vertex1, vertex2);

		//std::thread thread1(lambdaTest);
		//std::thread thread2(lambdaTest);
		
		//thread1.join();
		//thread2.join();
	}

	template<typename FragmentInput> FragmentInput Lerp(FragmentInput const &from, FragmentInput const &to, float t)
	{
		FragmentInput res = from;
		res.pos += (to.pos - from.pos) * t;
		res.customData += (to.customData - from.customData) * t;

		return res;
	}
	
	template<typename FragmentInput> std::vector<FragmentInput> clipPolygon( const std::vector<FragmentInput> &clipSpacePolygon)
	{
		ASSERT(clipSpacePolygon.size() >= 3, "Invalid polygon");

		vector<FragmentInput> output;

		
		//clip polygon against near plane using Sutherland–Hodgman Algorithm
		for (size_t vIdx = 0; vIdx < clipSpacePolygon.size(); ++vIdx)
		{
			float curVal = clipSpacePolygon[vIdx].pos[2] + clipSpacePolygon[vIdx].pos[3];

			size_t nextVertIdx = (vIdx + 1) % clipSpacePolygon.size();
			float nextVal = clipSpacePolygon[nextVertIdx].pos[2] + clipSpacePolygon[nextVertIdx].pos[3];

			if (curVal > 0.0f) // vertex is in front of camera
			{
				output.push_back(clipSpacePolygon[vIdx]);

				if (nextVal < 0.0f) //vertex is behind camera -> we need to split this edge
				{
					float ratio = curVal / (curVal - nextVal);
					output.push_back(Lerp(clipSpacePolygon[vIdx], clipSpacePolygon[nextVertIdx], ratio));
				}
			}
			else //vertex is behind camera
			{
				if (nextVal > 0.0f) //vertex is in front of camera -> we need to split this edge
				{
					float ratio = curVal / (curVal - nextVal);
					output.push_back(Lerp(clipSpacePolygon[vIdx], clipSpacePolygon[nextVertIdx], ratio));
				}
			}
		}

		return output;
	}

	template<typename FragmentInput> std::vector<FragmentInput> clipTriangleAndTransformToWindowSpace( FragmentInput const &v0, FragmentInput const &v1, FragmentInput const &v2)
	{
		//vertex shader transforms vertices into clip space and then we clip the triangle if needed
		vector<FragmentInput> output = clipPolygon(std::vector<FragmentInput>{ m_VertexShader->process(v0) , m_VertexShader->process(v1), m_VertexShader->process(v2) });


		for (auto &it : output)
		{
			float w= it.pos[3];
			float invW = 1.0f / w;

			it.pos *= invW;

			//we are in NDC now
			
			it.pos = m_VertexShader->m_Uniforms.viewportMatrix * it.pos;

			//shift depth range to be in <0,1.0f>
			it.pos[2] = it.pos[2] * 0.5f + 0.5f;

			//preserve distance from camera
			it.pos[3] = w;
		}

		//assuming  Counter Clock Wise order of vertices
		if (gmtl::makeCross(ConvertToVec3(output[1].pos - output[0].pos), ConvertToVec3(output[2].pos - output[1].pos))[2] >= 0.0f)
		{
			//polygon is not facing camera
			return std::vector<FragmentInput>();
		}

		return output;
	}
	
	template<typename VertexBuffer, typename IndexBuffer> void renderRange(VertexBuffer const &vertexBuffer, IndexBuffer const &indexBuffer, size_t fromIdx, size_t toIdx)
	{
		for (size_t idx = fromIdx; idx < toIdx; idx += 3)
		{
			//transform polygon from model space into windows space, might involve polygon clipping if its partially out of screen
			auto windowSpacePolygon = clipTriangleAndTransformToWindowSpace(
				vertexBuffer[indexBuffer[idx]],
				vertexBuffer[indexBuffer[idx + 1]],
				vertexBuffer[indexBuffer[idx + 2]]);

			//triangulate polygon and rasterize it
			for (size_t vIdx = 2; vIdx < windowSpacePolygon.size(); ++vIdx)
			{
				rasterizeBarycentric(windowSpacePolygon[0], windowSpacePolygon[vIdx-1], windowSpacePolygon[vIdx]);
			}
		}
	}


	template<typename VertexBuffer, typename IndexBuffer> void render( VertexBuffer const &vertexBuffer, IndexBuffer const &indexBuffer)
	{
		ASSERT(m_Res[0] > 0 && m_Res[1] > 0, "Invalid resolution");
		ASSERT(indexBuffer.size() % 3 == 0, "Index buffer must consist of triplets");
		ASSERT(nullptr != m_VertexShader, "Vertex shader cant be null");
		ASSERT(nullptr != m_FragmentShader, "Fragment shader cant be null");
		ASSERT(nullptr != m_RasterShader, "Raster shader cant be null");

		size_t polyCnt = indexBuffer.size() / 3;

		/*
		size_t range1Limit = (polyCnt / 2) * 3;
		auto renderRange1 = [&, this] 
		{ 
			renderRange(vertexBuffer, indexBuffer, 0, range1Limit); 
		};

		auto renderRange2 = [&, this] 
		{ 
			renderRange(vertexBuffer, indexBuffer, range1Limit, indexBuffer.size());
		};
		
		std::thread thread2(renderRange2);

		renderRange1();
		thread2.join();
		*/

		renderRange(vertexBuffer, indexBuffer, 0, indexBuffer.size());
	}

private:

	unsigned int m_Res[2] = { 0,0 };
	VertexShader *m_VertexShader;
	FragmentShader *m_FragmentShader;
	RasterShader *m_RasterShader;
};


}