#ifndef AE_VOLUMETRIC_H
#define AE_VOLUMETRIC_H

#include "../System.h"
#include "../RenderTarget.h"
#include "../texture/Texture2D.h"

namespace Atlas {

    namespace Lighting {

        class Volumetric {

        public:
            Volumetric(int32_t sampleCount = 10, float intensity = 1.0f);

            int32_t sampleCount = 10;
            float intensity = 1.0f;

        };

    }

}

#endif