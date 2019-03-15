#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include "FrameBuffer.h"

class GaussianBlur {
public:
	static void Init();
	static void BlurAndRenderToScreen(FrameBuffer& frameBuffer, const float& scale, const float& blurScale, const float& sigma);

	static float GaussianDistribution(float x, float sigma);
private:

	static ShaderProgram Hblur5x5;
	static ShaderProgram Vblur5x5;
};