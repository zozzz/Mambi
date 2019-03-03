#pragma once
#include "stdafx.h"
#include "Color.h"
#include "Common.h"


namespace Mambi
{
	class Ambilight;

	enum SampleOrient {
		Horizontal = 1,
		Vertical = 2
	};


	enum SampleAlign {
		Begin = 1,
		End = 2
	};


	struct SampleDesc {
		DisplayDim HPadding;
		DisplayDim VPadding;
		DisplayDim Width;
		DisplayDim Height;
		UINT16 Count;

		inline bool operator==(const SampleDesc& other)
		{
			return Count == other.Count
				&& Height == other.Height
				&& Width == other.Width
				&& HPadding == other.Height
				&& VPadding == other.VPadding;
		}

		inline bool operator!=(const SampleDesc& other)
		{
			return Count != other.Count
				|| Height != other.Height
				|| Width != other.Width
				|| HPadding != other.Height
				|| VPadding != other.VPadding;
		}
	};	
	

	class DisplaySamples;


	class DisplaySample {
	public:
		friend DisplaySamples;

		DisplaySample(DisplayDim w, DisplayDim h, D3D11_BOX src) : Width(w), Height(h), Src(src) {};
		~DisplaySample() {};

		DisplayDim Width;
		DisplayDim Height;
		D3D11_BOX Src;
		rgb_t Avg;
		RECT Dst;
	};


	typedef std::vector<DisplaySample> DisplaySampleList;
	

	template<SampleOrient Orient>
	class DisplaySampleFactory 
	{
	public:
		friend DisplaySamples;
		inline DisplaySampleFactory(const SampleDesc& desc): _desc(desc) { }

		template<SampleAlign Align>
		const void Create(DisplaySampleList& samples, DisplayDim dWidth, DisplayDim dHeight, float scale) const
		{
			dWidth = (DisplayDim)floor(dWidth * scale);
			dHeight = (DisplayDim)floor(dHeight * scale);

			DisplayDim lwidth = (DisplayDim)floor(_desc.Width * scale);
			DisplayDim lheight = (DisplayDim)floor(_desc.Height * scale);

			DisplaySampleList _lsamples;
			if (Orient == SampleOrient::Horizontal)
			{
				dWidth -= (DisplayDim)floor(_desc.HPadding * scale * 2);
				int mcount = (int)floor(dWidth / lwidth);
				int count = min(_desc.Count, mcount);
				int gap = (int)floor((dWidth - (lwidth * count)) / (count - 1));

				DisplayDim x = (DisplayDim)floor(_desc.HPadding * scale);
				for (int i = 0; i < count; i++)
				{
					D3D11_BOX src;
					src.back = 1;
					src.front = 0;
					src.left = x;
					src.top = (DisplayDim)(Align == SampleAlign::Begin 
						? _desc.VPadding 
						: floor(dHeight - lheight) - _desc.VPadding);
					src.right = src.left + _desc.Width;
					src.bottom = src.top + _desc.Height;

					x += lwidth + gap;
					_lsamples.push_back(DisplaySample(lwidth, lheight, src));
				}

				if (Align == SampleAlign::End)
				{
					std::reverse(_lsamples.begin(), _lsamples.end());
				}
			}
			else if (Orient == SampleOrient::Vertical)
			{
				dHeight -= (DisplayDim)floor(_desc.VPadding * scale * 2);
				int mcount = (int)floor(dHeight / lheight);
				int count = min(_desc.Count, mcount);
				int gap = (int)floor((dHeight - (lheight * count)) / (count - 1));

				DisplayDim y = (DisplayDim)floor(_desc.VPadding * scale);
				for (int i = 0; i < count; i++)
				{
					D3D11_BOX src;
					src.back = 1;
					src.front = 0;
					src.left = (DisplayDim)(Align == SampleAlign::Begin 
						? _desc.HPadding 
						: floor(dWidth - lwidth) - _desc.HPadding);
					src.top = y;
					src.right = src.left + _desc.Width;
					src.bottom = src.top + _desc.Height;

					y += lheight + gap;
					_lsamples.push_back(DisplaySample(lwidth, lheight, src));
				}

				if (Align == SampleAlign::Begin)
				{
					std::reverse(_lsamples.begin(), _lsamples.end());
				}
			}

			for (auto& s : _lsamples)
			{
				samples.push_back(s);
			}
		}

		inline bool operator==(const DisplaySampleFactory& other) { return _desc == other._desc; }
		inline bool operator!=(const DisplaySampleFactory& other) { return _desc != other._desc; }

	private:
		SampleDesc _desc;		
	};


	class DisplaySamples
	{
	public:
		friend Ambilight;

		DisplaySamples(
			const DisplaySampleFactory<SampleOrient::Horizontal>& hFactory,
			const DisplaySampleFactory<SampleOrient::Vertical>& vFactory,
			int8_t rotate)
			: _hFactory(hFactory), _vFactory(vFactory), _rotate(rotate)
		{}

		~DisplaySamples()
		{}

		inline auto& Items() { return _samples; }
		inline auto Width() const { return _width; }
		inline auto Height() const { return _height; }
		inline auto HFactory() const { return _hFactory; }
		inline auto VFactory() const { return _vFactory; }
		inline auto Rotate() const { return _rotate; }

		void Update(DisplayDim nWidth, DisplayDim nHeight, DisplayDim dWidth, DisplayDim dHeight)
		{
			_samples.clear();
			_hFactory.Create<SampleAlign::Begin>(_samples, dWidth, dHeight, (float)(dWidth / nWidth));
			_vFactory.Create<SampleAlign::End>(_samples, dWidth, dHeight, (float)(dHeight / nHeight));
			_hFactory.Create<SampleAlign::End>(_samples, dWidth, dHeight, (float)(dWidth / nWidth));
			_vFactory.Create<SampleAlign::Begin>(_samples, dWidth, dHeight, (float)(dHeight / nHeight));
			UpdateDst();
			ApplyRotate();
		}		

	private:
		void ApplyRotate()
		{
			if (_rotate == 0)
			{
				return;
			}

			if (abs(_rotate) >= _samples.size())
			{
				throw new std::exception("LedStrip offset is out of range.");
			}

			DisplaySampleList tmp;

			if (_rotate < 0)
			{
				throw new std::exception("Negative offset is not supported currently, plase write to developer :).");
			}
			else
			{
				tmp.insert(tmp.begin(), _samples.end() - _rotate, _samples.end());
				_samples.erase(_samples.end() - _rotate, _samples.end());
				_samples.insert(_samples.begin(), tmp.begin(), tmp.begin() + _rotate);
			}
		}


		void UpdateDst()
		{
			DisplayDim x = 0;
			DisplayDim mh = 0;
			
			for (auto& s : _samples)
			{
				s.Dst.top = 0;
				s.Dst.left = x;
				s.Dst.right = x + s.Width;
				s.Dst.bottom = s.Height;
				mh = max(mh, s.Height);
				x += s.Width;
			}

			_width = x;
			_height = mh;
		}

		int8_t _rotate;
		DisplayDim _width;
		DisplayDim _height;
		DisplaySampleList _samples;
		DisplaySampleFactory<SampleOrient::Horizontal> _hFactory;
		DisplaySampleFactory<SampleOrient::Vertical> _vFactory;
	};

}