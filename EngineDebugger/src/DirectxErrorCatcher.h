#include <vector>
#include <wrl.h>
#include <dxgidebug.h>

namespace EngineDebug
{
	class __declspec(dllexport) DirectxErrorCatcher
	{
	public:
		DirectxErrorCatcher();
		~DirectxErrorCatcher() = default;
		DirectxErrorCatcher(const DirectxErrorCatcher&) = delete;
		DirectxErrorCatcher& operator = (const DirectxErrorCatcher&) = delete;
	public:
		void Set();
		std::vector<std::string> GetMessages() const;
	private:
		unsigned long long next = 0;
#pragma warning(push)
#pragma warning(disable:4251)
		Microsoft::WRL::ComPtr<IDXGIInfoQueue> pInfoQueue;
#pragma warning(pop)
	};
}