#pragma once


#define MAX_LOADSTRING 100


namespace Mambi 
{
	template<unsigned int ID>
	class LoadRes {
	public:
		static const unsigned int Id = ID;
		static inline const auto IntResource() { return MAKEINTRESOURCE(ID); }
	};


	template<unsigned int ID, unsigned int LEN = MAX_LOADSTRING>
	class StringRes : public LoadRes<ID> {
	public:
		inline StringRes()
		{
			int size = LoadStringW(Application::InstanceHandle(), ID, data, LEN);
			if (size == 0) {
				throw new std::exception("TODO: GetLastError.");
			}
		}

		inline const auto Data() const { return data; }

	private:
		WCHAR data[LEN];
	};


	template<unsigned int ID>
	class IconRes : public LoadRes<ID>
	{
	public:
		inline IconRes()
		{
			data = LoadIcon(Application::InstanceHandle(), MAKEINTRESOURCE(ID));
			if (data == NULL)
			{
				throw new std::exception("TODO: GetLastError.");
			}
		}

		inline const auto Data() const { return data; }

	private:
		HICON data;
	};
}