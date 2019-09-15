#pragma once
#include <Engine/Core/RenderingManager.h>

#include <Common/Maths/Ray.h>

#include <Common/StdInclude.h>
#include <Common/BLASingleton.h>
/*
TODO:
SEPARATE render m_debug, for a specific CONTEXT (when added multiple windows and context)
And Common m_debug asserts
*/

namespace BLAengine
{
	class BLACORE_API DebugDraw
	{
		BLA_DECLARE_SINGLETON(DebugDraw)

	public:

		DebugDraw(DebugRenderingManager* renderManager);
		~DebugDraw();

		void Update();

		static void DrawLine(const blaVec3& origin, const blaVec3& destination);
		static void DrawLine(const blaVec3& origin, const blaVec3& destination, const blaVec3& color);

		static void DrawRay(const Ray& ray);
		static void DrawRay(const Ray& ray, const blaVec3& color);
		
		static void DrawSphere(const blaVec3& origin, blaF32 radius);
		static void DrawSphere(const blaVec3& origin, blaF32 radius, const blaVec3& color);

		static void DrawAABB(const blaVec3& origin, const blaVec3& halfExtent, const blaVec3& color);
		static void DrawOBB(const blaPosQuat& transform, const blaVec3& halfExtent, const blaVec3& color);

		static void DrawBasis(const blaPosQuat& transform);
		static void DrawBasis(const blaPosQuat& transform, float opacity);
		static void DrawBasis(const blaPosQuat& transform, float size, float opacity);

		static void DrawPlainSphere(const blaVec3& origin, blaF32 radius, const blaVec4& colorAndAlpha, blaU32 resolution = 12);
		static void DrawPlainAABB(const blaVec3& origin, const blaVec3& halfExtent, const blaVec4& colorAndAlpha);
		static void DrawPlainOBB(const blaPosQuat& transform, const blaVec3& halfExtent, const blaVec4& colorAndAlpha);

	private:
		void InstanceDrawLine(const blaVec3& origin, const blaVec3& destination, const blaVec3& color);
			 
		void InstanceDrawRay(const Ray& ray, const blaVec3& color);
			 
		void InstanceDrawSphere(const blaVec3& origin, blaF32 radius, const blaVec3& color, blaU32 resolution = 12);

		void InstanceDrawPlainSphere(const blaVec3& origin, blaF32 radius, const blaVec4& colorAndAlpha, blaU32 resolution = 1000);

		void InstanceDrawAABB(const blaVec3& origin, const blaVec3& halfExtent, const blaVec3& color);

		void InstanceDrawPlainAABB(const blaVec3& origin, const blaVec3& halfExtent, const blaVec4& colorAndAlpha);

		void InstanceDrawOBB(const blaPosQuat& transform, const blaVec3& halfExtent, const blaVec3& color);

		void InstanceDrawPlainOBB(const blaPosQuat& transform, const blaVec3& halfExtent, const blaVec4& colorAndAlpha);

		void InstanceDrawBasis(const blaPosQuat& transform, float size, float opacity);

		blaPair<blaVector<blaVec3>, blaVector<blaVec3>> m_lineMeshVertsAndColor;
		blaPair<blaVector<blaVec3>, blaVector<blaVec4>> m_debugTrianglesVertsAndColorA;
		DebugRenderingManager* m_debugRenderManager;

		bool m_drawDebugLines;
		bool m_drawDebugMeshes;
	};

}