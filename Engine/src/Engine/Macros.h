#pragma once

//DLL Import-Export Macro
#ifdef ENGINE_BUILD_DLL
#define ENGINE_DLL_DS __declspec(dllexport)
#else
#define ENGINE_DLL_DS __declspec(dllimport)
#endif