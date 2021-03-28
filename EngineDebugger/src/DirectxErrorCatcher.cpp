#include "DirectxErrorCatcher.h"
#include "Exceptions/EngineWndHresultException.h"
#include "Exceptions/EngineGFXHresultException.h"
#include "win.h"

#include <memory>

#pragma comment(lib, "dxguid.lib")

namespace EngineDebug
{
	DirectxErrorCatcher::DirectxErrorCatcher()
	{
		typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

		const auto hModDxgiDebug = LoadLibraryEx("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (hModDxgiDebug == nullptr)
		{
			throw WND_LAST_EXCEPT();
		}

		const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(reinterpret_cast<void*>(GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface")));
		if (DxgiGetDebugInterface == nullptr)
		{
			throw WND_LAST_EXCEPT();
		}

		HRESULT hr;
		GFX_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &pInfoQueue));
	}

	void DirectxErrorCatcher::Set()
	{
		next = pInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
	}

	std::vector<std::string> DirectxErrorCatcher::GetMessages() const
	{
		std::vector<std::string> messages;
		const auto end = pInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		for (auto i = next; i < end; i++)
		{
			HRESULT hr;
			SIZE_T messageLength;
			GFX_THROW_NOINFO(pInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength));
			auto bytes = std::make_unique<byte[]>(messageLength);
			auto pMessage = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
			GFX_THROW_NOINFO(pInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, pMessage, &messageLength));
			messages.emplace_back(pMessage->pDescription);
		}
		return messages;
	}
}