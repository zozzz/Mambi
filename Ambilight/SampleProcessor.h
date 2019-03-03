#pragma once
#include "stdafx.h"
#include "DisplaySample.h"


#define MAMBI_SP_SSE 1

// https://www.compuphase.com/graphic/scale3.htm
#define MAMBI_SP_AVERAGE(a, b)   ( ((((a) ^ (b)) & 0xfffefefeL) >> 1) + ((a) & (b)) )



namespace Mambi
{
	class SampleProcessor
	{	

#if MAMBI_SP_SSE

#define MAMBI_SP_SSE_PIXEL_PER_CYCLE 4
#define MAMBI_SAMPLE_MIN_WIDTH       4
#define MAMBI_SAMPLE_MIN_HEIGHT      1

	public:		
		SampleProcessor() 
		{
			uint8_t _avgMask[16] = {
				0xFF, 0xFE, 0xFE, 0xFE,
				0xFF, 0xFE, 0xFE, 0xFE,
				0xFF, 0xFE, 0xFE, 0xFE,
				0xFF, 0xFE, 0xFE, 0xFE
			};
			mask = _mm_loadu_si128(reinterpret_cast<__m128i*>(_avgMask));
		}

		template<typename T>
		void UpdateAvarage(DisplaySamples* samples, T* data, size_t pitch)
		{
			pitch /= sizeof(T);
			DisplayDim x, y;
			uint32_t avg1, avg2;
			__m128i avarage;
			__m128i current;
			__m128i tmp;

			for (auto& sample : samples->Items())
			{
				avarage = _mm_setzero_si128();
				for (y = sample.Dst.top; y < sample.Dst.bottom; ++y)
				{
					for (x = sample.Dst.left; x < sample.Dst.right; x += MAMBI_SP_SSE_PIXEL_PER_CYCLE)
					{
						current = _mm_loadu_si128(reinterpret_cast<__m128i*>(data + (y * pitch + x)));
						tmp = _mm_xor_si128(avarage, current);
						tmp = _mm_and_si128(tmp, mask);
						tmp = _mm_srli_epi32(tmp, 1);
						avarage = _mm_and_si128(avarage, current);
						avarage = _mm_add_epi32(tmp, avarage);						
					}
				}	

				avg1 = MAMBI_SP_AVERAGE(avarage.m128i_u32[0], avarage.m128i_u32[1]);
				avg2 = MAMBI_SP_AVERAGE(avarage.m128i_u32[2], avarage.m128i_u32[3]);
				avg1 = MAMBI_SP_AVERAGE(avg1, avg2);
				sample.Avg.r = reinterpret_cast<T*>(&avg1)->r;
				sample.Avg.g = reinterpret_cast<T*>(&avg1)->g;
				sample.Avg.b = reinterpret_cast<T*>(&avg1)->b;
			}
		}

	private:
		__m128i mask;		

#else
#define MAMBI_SAMPLE_MIN_WIDTH       1
#define MAMBI_SAMPLE_MIN_HEIGHT      1

	public:
		template<typename T>
		void UpdateAvarage(DisplaySamples* samples, T* data, size_t pitch)
		{
			pitch /= sizeof(T);
			uint32_t avg;
			uint32_t inp;

			for (auto& sample : samples->Items())
			{
				avg = 0;
				for (DisplayDim y = sample.Dst.top; y < sample.Dst.bottom; ++y)
				{
					for (DisplayDim x = sample.Dst.left; x < sample.Dst.right; ++x)
					{
						inp = reinterpret_cast<uint32_t*>(data)[y * pitch + x];
						avg = MAMBI_SP_AVERAGE(avg, inp);						
					}
				}

				sample.Avg.r = reinterpret_cast<T*>(&avg)->r;
				sample.Avg.g = reinterpret_cast<T*>(&avg)->g;
				sample.Avg.b = reinterpret_cast<T*>(&avg)->b;
			}
		}
#endif
	};
}