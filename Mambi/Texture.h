#pragma once


namespace Mambi
{
	template<typename T>
	class TextureData
	{
	public:
		TextureData(const T* data, const uint32_t pitch) : _data(data), _pitch(pitch / sizeof(T)) {};
		~TextureData() {};

		inline const T& At(uint32_t x, uint32_t y)
		{
			return _data[_pitch * y + x]; // row by row
		}

	private:
		const T* _data;
		const uint32_t _pitch;
	};


	class MappedTexture
	{
	public:
		MappedTexture(ID3D11Texture2D* _texture, D3D11_TEXTURE2D_DESC& _desc);
		~MappedTexture();

		template<typename T>
		inline std::shared_ptr<TextureData<T>> Data() {
			return std::shared_ptr<TextureData<T>>(new TextureData<T>(reinterpret_cast<T*>(_map.pData), _map.RowPitch));
		}

	private:
		D3D11_MAPPED_SUBRESOURCE _map;
		ID3D11Texture2D* _mapped;
		ID3D11Texture2D* _tmp;
		ID3D11Device* _device;
		ID3D11DeviceContext* _context;
	};


	class Texture
	{
	public:
		Texture(ID3D11Texture2D* texture) :_texture(texture) 
		{
			texture->GetDesc(&_desc);
		};

		Texture(ID3D11Texture2D* texture, D3D11_TEXTURE2D_DESC desc) :_texture(texture), _desc(desc) {};

		~Texture()
		{
			if (_texture != NULL) 
			{
				_texture->Release();
				_texture = NULL;
			}
		}

		inline auto& Desc() const { return _desc; };
		inline auto Width() const { return _desc.Width; }
		inline auto Height() const { return _desc.Height; }
		
		std::shared_ptr<MappedTexture> Map();

	private:
		ID3D11Texture2D* _texture;
		D3D11_TEXTURE2D_DESC _desc;
	};
	
}



