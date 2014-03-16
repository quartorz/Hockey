#pragma once

#include <Windows.h>
#include <wincodecsdk.h>
#include <d2d1.h>
#include "Macro.h"
#include <comdef.h>

_COM_SMARTPTR_TYPEDEF(IWICBitmapDecoder, __uuidof(IWICBitmapDecoder));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameDecode, __uuidof(IWICBitmapFrameDecode));
_COM_SMARTPTR_TYPEDEF(IWICFormatConverter, __uuidof(IWICFormatConverter));

// ‰æ‘œ‚ð“Ç‚Ýž‚Þ‚½‚ß‚ÌƒNƒ‰ƒX
class Decoder{
	IWICImagingFactory *factory;
public:
	Decoder(): factory(NULL)
	{
		::CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&factory));
	}
	~Decoder()
	{
		factory->Release();
	}
	bool Decode(LPCWSTR file,ID2D1RenderTarget *target,ID2D1Bitmap **bmp)
	{
		using namespace _com_util;
		try{
			IWICBitmapDecoderPtr decoder;
			CheckError(factory->CreateDecoderFromFilename(file,NULL,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&decoder));

			IWICBitmapFrameDecodePtr frame;
			CheckError(decoder->GetFrame(0,&frame));

			IWICFormatConverterPtr conv;
			CheckError(factory->CreateFormatConverter(&conv));

			CheckError(conv->Initialize(frame,GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,NULL,0.f,WICBitmapPaletteTypeMedianCut));
			CheckError(target->CreateBitmapFromWicBitmap(conv,bmp));
		}catch(_com_error &e){
			::MessageBoxW(NULL, e.ErrorMessage(), NULL, MB_OK);
			return false;
		}
		return true;
	}
};
