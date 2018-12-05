#include "PointLight.h"
#include "../GameObject.h"

using namespace BLAengine;

PointLight::PointLight():
    m_lightPosition(blaVec3(0)),m_lightIntensity(1.f),m_lightRadius(1.f)
{}

PointLight::PointLight(blaVec3 position,float radius, float intensity)
{
    m_lightPosition = position;
    m_lightRadius = radius;
    m_lightIntensity = intensity;
}

void PointLight::Update()
{

}