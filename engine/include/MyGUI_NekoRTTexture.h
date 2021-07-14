#pragma once

#include "MyGUI/MyGUI_Prerequest.h"
#include "MyGUI/MyGUI_ITexture.h"
#include "MyGUI/MyGUI_RenderFormat.h"
#include "MyGUI/MyGUI_IRenderTarget.h"

namespace MyGUI {

  class NekoRTTexture: public IRenderTarget {
  public:
    NekoRTTexture( unsigned int _texture );
    ~NekoRTTexture() override;

    void begin() override;
    void end() override;

    void doRender( IVertexBuffer* _buffer, ITexture* _texture, size_t _count ) override;

    const RenderTargetInfo& getInfo() const override
    {
      return mRenderTargetInfo;
    }

  private:
    RenderTargetInfo mRenderTargetInfo;
    unsigned int mTextureId;
    int mWidth;
    int mHeight;

    int mSavedViewport[4];

    unsigned int mFBOID;
    unsigned int mRBOID;
  };

}
