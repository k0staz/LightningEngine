#pragma once

#include <dxgi.h>

#include "CoreMinimum.h"

namespace LE::D3D11
{
void VerifyD3D11Result(HRESULT Result, const char* Code, const char* Filename, uint32 Line);

#define VERIFYD3D11RESULT(x) {HRESULT hr = x; if(FAILED(hr)) {VerifyD3D11Result(hr, #x, __FILE__, __LINE__);}}
}
