#pragma once
#include "Color.h"


namespace Mambi
{
	typedef UINT DisplayDim;


	enum SampleOrient {
		Horizontal = 1,
		Vertical = 2
	};


	enum SampleAlign {
		Begin = 1,
		End = 2
	};


	class DisplaySamples;

	
	class DisplaySample {
	public:
		friend DisplaySamples;

		DisplaySample(DisplayDim w, DisplayDim h, D3D11_BOX src) : _w(w), _h(h), _src(src) {};
		~DisplaySample() {};

		inline auto& Src() const { return _src; }
		inline auto& Dst() const { return _dst; }
		inline auto& Width() const { return _w; }
		inline auto& Height() const { return _h; }
		// inline rgb_t* Avg() { return &_avg; }

		template<typename T>
		void Process(T* data, UINT pitch)
		{
		}
		
		rgb_t avg;

	private:
		DisplayDim _w;
		DisplayDim _h;
		D3D11_BOX _src;
		RECT _dst;		
	};


	typedef std::vector<DisplaySample> DSL;
	class DisplaySamples;


	template<SampleOrient Orient>
	class DisplaySampleFactory {
	public:
		inline DisplaySampleFactory()
			: _width(0), _height(0), _margin(0) { }

		inline DisplaySampleFactory(DisplayDim w, DisplayDim h, DisplayDim m)
			: _width(w), _height(h), _margin(m) { }


		template<SampleAlign Align>
		const void Create(DSL& samples, DisplayDim dWidth, DisplayDim dHeight, float scale, UINT16 rcount) const
		{
			dWidth = (DisplayDim)floor(dWidth * scale);
			dHeight = (DisplayDim)floor(dHeight * scale);
			
			DisplayDim lwidth = (DisplayDim)floor(_width * scale);
			DisplayDim lheight = (DisplayDim)floor(_height * scale);

			DSL _lsamples;
			if (Orient == SampleOrient::Horizontal)
			{
				dWidth -= (DisplayDim)floor(_margin * scale * 2);
				int mcount = (int)floor(dWidth / lwidth);
				int count = min(rcount, mcount);
				int gap = (int)floor((dWidth - (lwidth * count)) / (count - 1));

				DisplayDim x = (DisplayDim)floor(_margin * scale);
				for (int i = 0; i < count; i++)
				{
					D3D11_BOX src;
					src.back = 1;
					src.front = 1;
					src.left = x;
					src.top = (DisplayDim)(Align == SampleAlign::Begin ? 0 : floor(dHeight - lheight));
					src.right = src.left + _width;
					src.bottom = src.top + _height;
					
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
				dHeight -= (DisplayDim)floor(_margin * scale * 2);
				int mcount = (int)floor(dHeight / lheight);
				int count = min(rcount, mcount);
				int gap = (int)floor((dHeight - (lheight * count)) / (count - 1));
				
				DisplayDim y = (DisplayDim)floor(_margin * scale);
				for (int i = 0; i < count; i++)
				{
					D3D11_BOX src;
					src.back = 1;
					src.front = 1;
					src.left = (DisplayDim)(Align == SampleAlign::Begin ? 0 : floor(dWidth - lwidth));
					src.top = y;
					src.right = src.left + _width;
					src.bottom = src.top + _height;

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
		
		inline bool operator==(const DisplaySampleFactory& other)
		{
			return other._width == _width && other._height == _height && other._margin == _margin;
		}
		inline bool operator!=(const DisplaySampleFactory& other)
		{
			return other._width != _width || other._height != _height || other._margin != _margin;
		}

	private:
		DisplayDim _width;
		DisplayDim _height;
		DisplayDim _margin;		
	};


	class DisplaySamples
	{
	public:
		DisplaySamples() 
		{}

		DisplaySamples(
			DisplaySampleFactory<SampleOrient::Horizontal>& hFactory,
			DisplaySampleFactory<SampleOrient::Vertical>& vFactory)
			: _hFactory(hFactory), _vFactory(vFactory) 
		{}

		~DisplaySamples() 
		{}

		inline auto& Items() { return _samples; }
		inline auto Width() const { return _width; }
		inline auto Height() const { return _height; }

		void Update(DisplayDim dWidth, DisplayDim dHeight, DisplayDim nWidth, DisplayDim nHeight, UINT16 hcount, UINT16 vcount)
		{
			_samples.clear();
			_hFactory.Create<SampleAlign::Begin>(_samples, dWidth, dHeight, (float)(dWidth / nWidth), hcount);
			_vFactory.Create<SampleAlign::End>(_samples, dWidth, dHeight, (float)(dHeight / nHeight), vcount);
			_hFactory.Create<SampleAlign::End>(_samples, dWidth, dHeight, (float)(dWidth / nWidth), hcount);
			_vFactory.Create<SampleAlign::Begin>(_samples, dWidth, dHeight, (float)(dHeight / nHeight), vcount);
			UpdateDst();

			//Console::WriteLine("DisplaySamples (%d x %d)", _width, _height);
		}

		void Move(INT16 offset)
		{
			if (offset == 0)
			{
				return;
			}

			if (offset < 0)
			{

			}
			else
			{
			}
		}

	private:
		void UpdateDst() 
		{
			DisplayDim x = 0;
			DisplayDim mh = 0;

			for (auto& s: _samples)
			{
				s._dst.top = 0;
				s._dst.left = x;
				s._dst.right = x + s._w;
				s._dst.bottom = s._h;
				mh = max(mh, s._h);
				x += s._w;
			}

			_width = x;
			_height = mh;
		}

		DisplayDim _width;
		DisplayDim _height;
		DSL _samples;
		DisplaySampleFactory<SampleOrient::Horizontal> _hFactory;
		DisplaySampleFactory<SampleOrient::Vertical> _vFactory;
	};

}