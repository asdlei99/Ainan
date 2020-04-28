#pragma once

#include "editor/InterpolationSelector.h"
#include "editor/CurveEditor.h"
#include "environment/ExposeToJson.h"

namespace Ainan {

	class ScaleCustomizer
	{
	public:
		ScaleCustomizer();
		void DisplayGUI();

		InterpolationSelector<float>& GetScaleInterpolator();
		CurveEditor m_Curve;

	private:
		//starting scale
		bool m_RandomScale = true;
		float m_DefinedScale = 2.0f;
		float m_MinScale = 20.0f;
		float m_MaxScale = 25.0f;

		//scale over time
		InterpolationSelector<float> m_Interpolator;
		float m_EndScale = m_DefinedScale;

		//random number generator
		std::mt19937 mt;

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}