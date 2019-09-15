#include "AnimationComponent.h"
#include "AssetsImport/ExternalFormats/BVHImport.h"
#include "Engine/EngineInstance.h"
#include "Engine/System/InputManager.h"
#include "Engine/Core/Timer.h"
#include "Engine/Core/DebugDraw.h"

using namespace BLAengine;

BEGIN_COMPONENT_DESCRIPTION(AnimationComponent)
EXPOSE(m_play)
EXPOSE(m_frameIndex)
EXPOSE(m_bvhName)
EXPOSE(m_playbackMultiplier)
END_DESCRIPTION()

AnimationComponent::AnimationComponent(GameObjectReference parentObject) :
    GameComponent(parentObject), 
	m_play(false), 
	m_frameIndex(0.f), 
    m_animation(nullptr),
	m_lastTimePlayerInteraction(0),
	m_playbackMultiplier(1.f)
{}

AnimationComponent::~AnimationComponent()
{
    delete m_animation;
}

void AnimationComponent::Update()
{
	InputManager* inputs = InputManager::GetSingletonInstance();
	if (m_animation == nullptr)
	{
		m_animation = BVHImport::ImportAnimation("./resources/animations/bvh/01_04.bvh")[0];
	}
	else
	{
		blaF32 animDt = 1.0f / m_animation->GetSamplingRate();
		const EngineInstance* engine = EngineInstance::GetSingletonInstanceRead();

		blaF32 gameDt = Timer::GetSingletonInstance()->GetDelta();
		blaF32 animStep = m_playbackMultiplier * (gameDt / animDt);

		if (inputs->GetKeyState(BLA_KEY_RIGHT).IsDown() || inputs->GetKeyState(BLA_KEY_LEFT).IsDown())
		{
			m_play = false;
		}

		if (m_play)
		{
			m_frameIndex += animStep;
		}
		else
		{
			if (inputs->GetKeyState(BLA_KEY_RIGHT).IsDown())
			{
				m_frameIndex += animStep;
				m_lastTimePlayerInteraction = Timer::GetSingletonInstance()->GetTime();
			}
			if (inputs->GetKeyState(BLA_KEY_LEFT).IsDown())
			{
				m_frameIndex -= animStep;
				m_frameIndex = m_frameIndex < 0.f ? 0.f : m_frameIndex;
				m_lastTimePlayerInteraction = Timer::GetSingletonInstance()->GetTime();
			}
		}

		m_frameIndex = m_frameIndex >= m_animation->GetFrameCount() ? 0.f : m_frameIndex;

		blaVector<blaPosQuat> jointTransformsW;
		m_animation->EvaluateAnimation(static_cast<int>(m_frameIndex), jointTransformsW);

		/*for (auto jointW : jointTransformsW)
		{
			DebugDraw::DrawBasis(jointW, 1.f);
		}*/

		blaVector<blaPair<blaPosQuat, blaF32>> bones;

		SkeletonAnimationData::GetBoneArrayFromEvalAnim(bones, m_animation->GetSkeleton(), jointTransformsW);

		blaF32 scaleFactor = 0.1f;

		for (size_t i = 0; i < bones.size(); ++i)
		{
			blaPosQuat t = bones[i].first;
			blaF32 h = bones[i].second;

			t.SetTranslation3(scaleFactor * t.GetTranslation3());
			h *= scaleFactor;

			DebugDraw::DrawPlainOBB(t, blaVec3(0.5f * h, 0.1f, 0.1f), blaVec4(BLAColors::ORANGE, 0.6f));
			DebugDraw::DrawOBB(t, blaVec3(0.5f * h, 0.1f, 0.1f), blaVec3(0));
		}
	}

	if(m_bvhName != m_prevBVHName)
	{
		delete m_animation;
		m_animation = BVHImport::ImportAnimation(m_bvhName)[0];
	}

	m_prevBVHName = m_bvhName;
}