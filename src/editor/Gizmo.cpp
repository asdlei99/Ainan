#include <pch.h>

#include "Gizmo.h"

#include <GLFW/glfw3.h>

namespace Ainan {

#define GIZMO_LINE_LENGTH_TILL_TIP 1.0f
#define GIZMO_LINE_LENGTH_TILL_WINGS 0.9f
#define GIZMO_LINE_WIDTH 0.075f
#define GIZMO_LINE_WINGS_HEIGHT 0.1f
#define GIZMO_LINE_START_X 0
#define GIZMO_LINE_START_Y 0

	//                                            v3
	//                                            |\
	//                                            | \
	//                                            |  \
	//           v1                             v2|   \
	//            |-------------------------------|    \
	//            |                                     \
	//            |                                      | v4
	//            |                                     /
	//            |-------------------------------|    /
	//           v0                             v6|   /
	//                                            |  /
	//                                            | /
	//                                            |/
	//                                            v5
	//

	//in normalized device coordinates -> ranges from -1 to 1
	static const glm::vec2 arrowVertices[] =
	{
		glm::vec2(GIZMO_LINE_START_X, GIZMO_LINE_START_Y),  //v0
		glm::vec2(GIZMO_LINE_START_X,  GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH),  //v1
		glm::vec2(GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS,  GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH),  //v2
		glm::vec2(GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS, GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH + GIZMO_LINE_WINGS_HEIGHT),  //v3
		glm::vec2(GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_TIP,  GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH / 2),  //v4
		glm::vec2(GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS, GIZMO_LINE_START_Y - GIZMO_LINE_WINGS_HEIGHT),  //v5
		glm::vec2(GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS, GIZMO_LINE_START_Y),  //v6

	};

	static const unsigned int arrowIndecies[] =
	{
		0,1,2,
		2,3,4,
		4,5,6,
		6,4,2,
		0,2,6
	};

	Gizmo::Gizmo()
	{
		VAO = Renderer::CreateVertexArray();
		VAO->Bind();

		VBO = Renderer::CreateVertexBuffer((void*)arrowVertices, sizeof(arrowVertices));
		VBO->Bind();
		VBO->SetLayout({ ShaderVariableType::Vec2 });

		EBO = Renderer::CreateIndexBuffer((unsigned int*)arrowIndecies, sizeof(arrowIndecies) / sizeof(unsigned int));
		EBO->Bind();

		VAO->Unbind();
	}

	void Gizmo::Draw(glm::vec2& objectPosition, const Rectangle& viewport)
	{
		double xpos, ypos;
		glfwGetCursorPos(Window::Ptr, &xpos, &ypos);

		//change from being relative to top left to being relative to bottom left
		ypos = -ypos + Window::Size.y;

		if (xpos < viewport.X || xpos > viewport.X + viewport.Width ||
			ypos < viewport.Y || ypos > viewport.Y + viewport.Height)
		{
			xpos = -1000000;
			ypos = -1000000;
		}

		//change from being relative to the bottom left of the screen
		//to being relative to bottom left of the viewport
		xpos -= viewport.X;
		ypos -= viewport.Y;

		float NDC_xpos = (float)xpos * 2 / viewport.Width - 1.0f;
		float NDC_ypos = (float)ypos * 2 / viewport.Height - 1.0f;

		glm::vec2 objectPositionWS = objectPosition * c_GlobalScaleFactor;
		glm::vec2 realMousePositionNDC = glm::vec2(NDC_xpos, NDC_ypos);

		glm::mat4 invView = glm::inverse(Renderer::m_CurrentSceneDescription.SceneCamera.ViewMatrix);
		glm::mat4 invProj = glm::inverse(Renderer::m_CurrentSceneDescription.SceneCamera.ProjectionMatrix);

		glm::vec4 result = invView * invProj * glm::vec4(realMousePositionNDC.x, realMousePositionNDC.y, 0.0f, 1.0f);

		//check collision inside arrow rectangle(the part before the wings)

		bool mouseButtonDown = glfwGetMouseButton(Window::Ptr, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

		GizmoArrow mouseOver = None;

		if (CheckIfInsideArrow(Horizontal, objectPositionWS, glm::vec2(result.x,result.y)) )
		{
			mouseOver = Horizontal;
			if (mouseButtonDown && m_ArrowPressedMap[&objectPosition] != Vertical) {
				m_ArrowPressedMap[&objectPosition] = Horizontal;

				if (m_ArrowPressLocationDistanceFromArrowOriginMap[&objectPosition] == 0.0f)
					m_ArrowPressLocationDistanceFromArrowOriginMap[&objectPosition] = result.x - objectPositionWS.x;
			}
		}

		if (CheckIfInsideArrow(Vertical, objectPositionWS, glm::vec2(result.x, result.y)))
		{
			mouseOver = Vertical;
			if (mouseButtonDown && m_ArrowPressedMap[&objectPosition] != Horizontal) {
				m_ArrowPressedMap[&objectPosition] = Vertical;

				if(m_ArrowPressLocationDistanceFromArrowOriginMap[&objectPosition] == 0.0f)
					m_ArrowPressLocationDistanceFromArrowOriginMap[&objectPosition] = result.y - objectPositionWS.y;
			}
		}

		if (m_ArrowPressedMap[&objectPosition] == Horizontal)
			objectPosition.x = (result.x - m_ArrowPressLocationDistanceFromArrowOriginMap[&objectPosition]) / c_GlobalScaleFactor;
		else if(m_ArrowPressedMap[&objectPosition] == Vertical)
			objectPosition.y = (result.y - m_ArrowPressLocationDistanceFromArrowOriginMap[&objectPosition]) / c_GlobalScaleFactor;

		if (!mouseButtonDown) {
			m_ArrowPressedMap[&objectPosition] = None;
			m_ArrowPressLocationDistanceFromArrowOriginMap[&objectPosition] = 0.0f;
		}

		//Draw the triangles
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(objectPositionWS.x, objectPositionWS.y, 0.0f));
		model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));

		glm::vec4 color;

		//assign a brighter color if the mouse is over the arrow
		if (m_ArrowPressedMap[&objectPosition] == Horizontal || mouseOver == Horizontal)
			color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		else
			color = glm::vec4(0.75f, 0.0f, 0.0f, 1.0f);

		auto& shader = Renderer::ShaderLibrary["GizmoShader"];

		shader->SetUniformVec4("u_Color", color);
		shader->SetUniformMat4("u_Model", model);
		Renderer::Draw(*VAO, *shader, Primitive::Triangles, *EBO);

		model = glm::rotate(model, -PI / 2, glm::vec3(0.0f, 0.0f, -1.0f));

		//assign a brighter color if the mouse is over the arrow
		if (m_ArrowPressedMap[&objectPosition] == Vertical || mouseOver == Vertical)
			color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		else
			color = glm::vec4(0.0f, 0.75f, 0.0f, 1.0f);

		shader->SetUniformVec4("u_Color", color);
		shader->SetUniformMat4("u_Model", model);
		glm::vec2 mousePosWS = Renderer::m_CurrentSceneDescription.SceneCamera.Position + realMousePositionNDC * c_GlobalScaleFactor;
		glm::vec2 objectPosWS = objectPositionWS * c_GlobalScaleFactor;

		Renderer::Draw(*VAO, *shader, Primitive::Triangles, *EBO);
	}

	bool Gizmo::CheckIfInsideArrow(const GizmoArrow& arrow, const glm::vec2& arrowCentre, const glm::vec2& point)
	{

		//check if inside arrow rectangle, that is the shape (v0, v1, v2, v6) in the arrow in the start of this file
		if (arrow == Horizontal)
		{
			if(point.x > arrowCentre.x&& point.x < arrowCentre.x + GIZMO_LINE_LENGTH_TILL_WINGS * c_GlobalScaleFactor / 10.0f &&
				   point.y > arrowCentre.y && point.y < arrowCentre.y + GIZMO_LINE_WIDTH * c_GlobalScaleFactor / 10.0f)
				return true;
		}
		else if (arrow == Vertical)
		{
			if(point.x > arrowCentre.x - GIZMO_LINE_WIDTH * c_GlobalScaleFactor / 10.0f && point.x < arrowCentre.x &&
				   point.y > arrowCentre.y && point.y < arrowCentre.y + GIZMO_LINE_LENGTH_TILL_WINGS * c_GlobalScaleFactor / 10.0f)
				return true;
		}
		//check if inside arrow triangle, that is the shape (v3, v4, v5) in the arrow in the start of this file
		glm::vec2 v3 = arrowVertices[3] / 10.0f;
		glm::vec2 v4 = arrowVertices[4] / 10.0f;
		glm::vec2 v5 = arrowVertices[5] / 10.0f;

		if (arrow == Horizontal)
		{
			//A,B,C represent the edges of the triangle
			glm::vec2 A = arrowCentre / c_GlobalScaleFactor + v3;
			glm::vec2 B = arrowCentre / c_GlobalScaleFactor + v4;
			glm::vec2 C = arrowCentre / c_GlobalScaleFactor + v5;


			glm::vec2 vector0 = C - A;
			glm::vec2 vector1 = B - A;
			glm::vec2 vector2 = point / c_GlobalScaleFactor - A;

			// Compute dot products
			float dot00 = glm::dot(vector0, vector0);
			float dot01 = glm::dot(vector0, vector1);
			float dot02 = glm::dot(vector0, vector2);
			float dot11 = glm::dot(vector1, vector1);
			float dot12 = glm::dot(vector1, vector2);

			// Compute barycentric coordinates
			float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
			float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
			float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

			// Check if point is in triangle
			return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
		}
		else if (arrow == Vertical)
		{
			{
				//rotate the triangle 90 degrees (pi / 2 in radians) because we are checking the vertical arrow
				glm::vec2 tempv3 = v3;
				glm::vec2 tempv4 = v4;
				glm::vec2 tempv5 = v5;

				float angle = PI / 2.0f;

				//rotate A
				v3.x = tempv3.x * cos(angle) - tempv3.y * sin(angle);
				v3.y = tempv3.x * sin(angle) + tempv3.y * cos(angle);
				//rotate B
				v4.x = tempv4.x * cos(angle) - tempv4.y * sin(angle);
				v4.y = tempv4.x * sin(angle) + tempv4.y * cos(angle);
				//rotate C
				v5.x = tempv5.x * cos(angle) - tempv5.y * sin(angle);
				v5.y = tempv5.x * sin(angle) + tempv5.y * cos(angle);
			}

			//A,B,C represent the edges of the triangle
			glm::vec2 A = arrowCentre / c_GlobalScaleFactor + v3;
			glm::vec2 B = arrowCentre / c_GlobalScaleFactor + v4;
			glm::vec2 C = arrowCentre / c_GlobalScaleFactor + v5;


			glm::vec2 vector0 = C - A;
			glm::vec2 vector1 = B - A;
			glm::vec2 vector2 = point / c_GlobalScaleFactor - A;

			// Compute dot products
			float dot00 = glm::dot(vector0, vector0);
			float dot01 = glm::dot(vector0, vector1);
			float dot02 = glm::dot(vector0, vector2);
			float dot11 = glm::dot(vector1, vector1);
			float dot12 = glm::dot(vector1, vector2);

			// Compute barycentric coordinates
			float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
			float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
			float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

			// Check if point is in triangle
			return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
		}

		return false;
	}

}