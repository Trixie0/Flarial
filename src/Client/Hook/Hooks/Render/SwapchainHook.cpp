#include "SwapchainHook.hpp"
#include "../../../GUI/D2D.hpp"
#include "../../../Events/Render/RenderEvent.hpp"
#include "../../../Events/EventHandler.hpp"
#include "d2d1_1.h"

SwapchainHook::SwapchainHook() : Hook("swapchain_hook", "") {}

void SwapchainHook::enableHook()
{
    auto swapchain_ptr = (void *)kiero::getMethodsTable()[140];
    this->manualHook(swapchain_ptr, swapchainCallback, (void **)&func_original);
}

bool init = false;
bool d2d = false;

void SwapchainHook::swapchainCallback(IDXGISwapChain *pSwapChain, UINT syncInterval, UINT flags)
{

    if(!init) {

        ID3D12Device5 *device;
        if (SUCCEEDED(pSwapChain->GetDevice(IID_PPV_ARGS(&device)))) {

            Logger::debug("Removing D3D12 Device");
            device->RemoveDevice();
            device->Release();
            device = nullptr;
        }


        ID3D11Device *d3d11device = nullptr;

        if (SUCCEEDED(pSwapChain->GetDevice(IID_PPV_ARGS(&d3d11device)))) {

            // Create the D2D factory
            ID2D1Factory* factory;
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);

            // Set up the D2D render target using the back buffer
            IDXGISurface* dxgiBackbuffer;
            pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackbuffer));
            factory->CreateDxgiSurfaceRenderTarget(dxgiBackbuffer, D2D1::RenderTargetProperties(
                    D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)), &D2D::context);


            dxgiBackbuffer->Release();
            dxgiBackbuffer = nullptr;

            factory->Release();
            factory = nullptr;

            init = true;
        }

    } else {

        if(D2D::context != nullptr) {

            MC::windowSize.x = D2D::context->GetSize().width;
            MC::windowSize.y = D2D::context->GetSize().height;

            D2D::context->BeginDraw();
            RenderEvent event;
            EventHandler::onRender(event);
            D2D::context->EndDraw();
        }

    }


    return func_original(pSwapChain, syncInterval, flags);


}