// BLAEngine Copyright (C) 2016-2020 Vincent Petrella. All rights reserved.

#include "InspectableVariablesGuiElements.h"
#include "Maths/Maths.h"
#include "Core/GameObject.h"

using namespace BLA;

InspectableVariablesEditorGuiElementFactoryManager::Factories InspectableVariablesEditorGuiElementFactoryManager::ms_factories;

void InspectableVariablesEditorGuiElementFactoryManager::__RegisterFactory(blaStringId type, GetEditorFactory factory)
{
	ms_factories.insert(std::make_pair(type, factory));
}

struct InspectableVariablesEditorGuiElementFactoryRegistrator
{
	InspectableVariablesEditorGuiElementFactoryRegistrator(blaStringId typeId, InspectableVariablesEditorGuiElementFactoryManager::GetEditorFactory factory)
	{
		InspectableVariablesEditorGuiElementFactoryManager::__RegisterFactory(typeId, factory);
	}
};

template<typename T>
DevGuiElement* MakeEditGuiElement(const blaString& variableName, blaStringId groupId, blaLambda<void(const char*, const char*, blaSize)> onEditFunctor, void* obj)
{
	return new DevGuiEditElement<T>(variableName, groupId, onEditFunctor, static_cast<T*>(obj));
}

template<typename T>
DevGuiElement* MakeVectorEditGuiElement(const blaString& variableName, blaStringId groupId, blaLambda<void(const char*, const char*, blaSize)> onEditFunctor, void* obj)
{
	return new DevGuiEditElementVector<T>(variableName, groupId, static_cast<blaVector<T>*>(obj));
}

#define RegisterInspectableVariableGuiElementEditorFactory(Type) \
	InspectableVariablesEditorGuiElementFactoryRegistrator g_TypeGuiElementEditorFactory##Type##Registartor(Core::InspectableVariables::TypeResolver<Type>::GetDescriptor()->m_typeID, MakeEditGuiElement<Type>);	\
	InspectableVariablesEditorGuiElementFactoryRegistrator g_TypeGuiElementEditorFactory##Type##VectorRegistartor(Core::InspectableVariables::TypeResolver<blaVector<Type>>::GetDescriptor()->m_typeID, MakeVectorEditGuiElement<Type>);

template<>
void DevGuiEditElementVector<blaBool>::Render()
{
    if (DevGuiEditElementVectorPreRender(this))
    {
        for (int i = 0; i < m_pToVector->size(); i++)
        {
            struct boxedBool
            {
                bool v;
            };
            boxedBool b = { m_pToVector->at(i) };
			DevGuiEditElement<blaBool> toRender(std::to_string(i), m_groupId, [](const char*, const char*, blaSize) {}, &b.v);
            toRender.Render();

            (*m_pToVector)[i] = b.v;
        }
        DevGuiEditElementVectorPostRender(this);
    }
}

template<>
BLA::Core::InspectableVariables::blaVectorDescriptor<blaBool>::blaVectorDescriptor() :
ExposedVarTypeDescriptor
{ GenerateBlaStringId(blaString("Vector<") + blaString(TypeResolver<blaBool>::GetDescriptor()->m_typeID) + ">"), sizeof(blaVector<blaBool>) },
m_itemType{ TypeResolver<blaBool>::GetDescriptor() }
{
	getSize = [](const void* vecPtr) -> blaSize
	{
		return static_cast<const blaVector<blaBool>*>(vecPtr)->size();
	};
	getItem = [](const void* vecPtr, blaSize index) -> const void*
	{
		return nullptr;
	};
}

RegisterInspectableVariableGuiElementEditorFactory(blaBool);
RegisterInspectableVariableGuiElementEditorFactory(blaS32);
RegisterInspectableVariableGuiElementEditorFactory(blaF32);
RegisterInspectableVariableGuiElementEditorFactory(blaString);
RegisterInspectableVariableGuiElementEditorFactory(blaVec2)
RegisterInspectableVariableGuiElementEditorFactory(blaVec3)
RegisterInspectableVariableGuiElementEditorFactory(blaQuat)
RegisterInspectableVariableGuiElementEditorFactory(blaPosQuat)
RegisterInspectableVariableGuiElementEditorFactory(blaScaledTransform);

using namespace Core;
RegisterInspectableVariableGuiElementEditorFactory(GameObject);

