#include <pch.h>

#include "ViewportWindow.h"

namespace Ainan {

	void ViewportWindow::DisplayGUI()
	{
		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.0f);
		ImGui::Begin("Viewport",nullptr ,ImGuiWindowFlags_NoBackground);
		ImGui::PopStyleVar();

		RenderViewport.X = ImGui::GetWindowPos().x;

		RenderViewport.Y = Window::FramebufferSize.y - (ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y + ImGui::GetFrameHeightWithSpacing());

		RenderViewport.Width = ImGui::GetWindowSize().x;
		RenderViewport.Height = ImGui::GetWindowSize().y;

		RenderViewport.X -= Window::Position.x;
		RenderViewport.Y += Window::Position.y; //negative because y axis is inverted in screen coordinates

		ImGui::End();
	}

}