// BLAEngine Copyright (C) 2016-2020 Vincent Petrella. All rights reserved.

#pragma once
#include "BLAStringID.h"

#include "StdInclude.h"
#include "Core/InspectableVariables.h"
#include "BLASingleton.h"

#define VertexAttributes(...) __VA_ARGS__
#define UniformValues(...) __VA_ARGS__

#define DeclareRenderPass(Name, VertexAttributes, UniformValues, attachmentCount) \
   typedef BLA::RenderPass<COMPILE_TIME_CRC32_STR(#Name), attachmentCount, BLA::_RenderPassTemplateHelpers::RPIS<EXPAND(VertexAttributes)>, BLA::_RenderPassTemplateHelpers::RPIS<EXPAND(UniformValues)>> Name;

#define RegisterRenderPass(Name)                                                                                        \
    class Name##Registrator                                                                                             \
    {                                                                                                                   \
    public:                                                                                                             \
        Name##Registrator()                                                                                             \
        {                                                                                                               \
            RenderPassRegistry* registry = RenderPassRegistry::GetSingletonInstance();                                  \
            if(!registry)                                                                                               \
            {                                                                                                           \
                 registry = RenderPassRegistry::AssignAndReturnSingletonInstance(new RenderPassRegistry());             \
            }                                                                                                           \
            blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*> vas = GetRenderPassVADescriptors<Name>();     \
            blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*> uvs = GetRenderPassUVDescriptors<Name>();     \
            registry->__RegisterRenderPass(Name::ms_renderPassId, 1, vas, uvs);                                         \
        }                                                                                                               \
    };                                                                                                                  \
    static Name##Registrator ms_Name##Registrator;                                                  

namespace BLA
{
    class BLACORE_API ShaderAttachment
    {

    };

    namespace _RenderPassTemplateHelpers
    {
        // Dummy class to serve as Render Pass Input Type Separator (used to separate lists of variadic template arguments)
        template<typename...>
        class RPIS {};
    }

    // Only inspectable types should be template arguments to a render pass.
    // Check and use it to infer things
    //
    template<typename... Ts>
    class _RenderPassInstanceVAs
    {
    public:
        _RenderPassInstanceVAs() {}
    };

    template<typename... Ts>
    class _RenderPassInstanceUVs
    {
    public:
        _RenderPassInstanceUVs<Ts...>() {};
    };

    namespace _RenderPassTemplateHelpers
    {
        template <size_t, class> struct InferVAType;

        template <typename T, typename... Ts>
        struct InferVAType<0, _RenderPassInstanceVAs<T, Ts...>>
        {
            typedef T VAType;
        };

        template <size_t k, typename T, typename... Ts>
        struct InferVAType<k, _RenderPassInstanceVAs<T, Ts...>>
        {
            typedef typename InferVAType<k - 1, _RenderPassInstanceVAs<Ts...>>::VAType VAType;
        };

        template <size_t, class> struct InferUVType;

        template <class T, class... Ts>
        struct InferUVType<0, _RenderPassInstanceUVs<T, Ts...>>
        {
            typedef T type;
        };

        template <size_t k, class T, class... Ts>
        struct InferUVType<k, _RenderPassInstanceUVs<T, Ts...>>
        {
            typedef typename InferUVType<k - 1, _RenderPassInstanceUVs<Ts...>>::type type;
        };
    }

    template<typename T, typename... Ts>
    class _RenderPassInstanceVAs<T, Ts...> : _RenderPassInstanceVAs<Ts...>
    {
    public:
        _RenderPassInstanceVAs(const blaVector<T>& dataVector, const blaVector<Ts>&... dataVectors) :
            _RenderPassInstanceVAs<Ts...>(dataVectors...),
            m_dataVector(dataVector)
        {
            // TODO: Reason about type, statically check inspectability... Pre-fecth type information
            // Remember this constructor is executed every time we request a new RenderObject for this RenderPass, so not like it's only registered once yea ?
        };

        template <size_t k>
        const blaVector<typename std::enable_if<k == 0, T>::type>& GetVertexAttributes()
        {
            return m_dataVector;
        }

        template <size_t k>
        const blaVector<typename std::enable_if<k != 0, typename _RenderPassTemplateHelpers::InferVAType<k, _RenderPassInstanceVAs<T, Ts...>>::VAType>
            ::type>& GetVertexAttributes()
        {
            return _RenderPassInstanceVAs<Ts...>::template GetVertexAttributes<k - 1>();
        }

    protected:
        const blaVector<T>& m_dataVector;
    };

    template<typename T, typename... Ts>
    class _RenderPassInstanceUVs<T, Ts...> : public _RenderPassInstanceUVs<Ts...>
    {
    public:
        _RenderPassInstanceUVs(const T& uniformValue, const Ts&... uniformValues) :
            _RenderPassInstanceUVs<Ts...>(uniformValues...),
            m_uniformValue(uniformValue)
        {
            // TODO: Reason about type, statically check inspectability... Pre-fecth type information
            // Remember this constructor is executed every time we request a new RenderObject for this RenderPass, so not like it's only registered once yea ?
        };

        template <size_t k>
        const typename std::enable_if<k == 0, T>::type& GetUniformValue()
        {
            return m_uniformValue;
        }

        template <size_t k>
        const typename std::enable_if<k != 0, typename _RenderPassTemplateHelpers::InferVAType<k, _RenderPassInstanceVAs<T, Ts...>>::VAType>
            ::type& GetUniformValue()
        {
            return _RenderPassInstanceVAs<Ts...>::template GetVertexAttributes<k - 1>();
        }

    protected:
        const T& m_uniformValue;
    };

    template<class VertexAttributes, class ShaderUniforms>
    class _RenderPassInstance;

    template<typename... VAs, typename... UVs>
    class _RenderPassInstance<_RenderPassTemplateHelpers::RPIS<VAs...>, _RenderPassTemplateHelpers::RPIS<UVs...>> : public _RenderPassInstanceVAs<VAs...>, public _RenderPassInstanceUVs<UVs...>
    {
    public:
        typedef _RenderPassInstanceVAs<VAs...> InstanceVertexAttributes;
        typedef _RenderPassInstanceUVs<UVs...> InstanceUniformValues;

        _RenderPassInstance(const InstanceVertexAttributes& vertexAttributes, const InstanceUniformValues& shaderUniforms ...) : _RenderPassInstanceVAs<VAs...>(vertexAttributes), _RenderPassInstanceUVs<UVs...>(shaderUniforms) {}
    };

    template<blaU32 renderPassId, blaS32 attachmentCount, class VertexAttributes, class ShaderUniforms>
    class RenderPass;

    template<blaU32 renderPassId, blaS32 attachmentCount, typename... VAs, typename... UVs>
    class BLACORE_API RenderPass<renderPassId, attachmentCount, _RenderPassTemplateHelpers::RPIS<VAs...>, _RenderPassTemplateHelpers::RPIS<UVs...>>
    {
        static_assert(attachmentCount > 0, "Number of attachments on a RenderPass < 1 is invalid");

        ShaderAttachment m_attachment[attachmentCount];

    public:
        typedef _RenderPassInstance<_RenderPassTemplateHelpers::RPIS<VAs...>, _RenderPassTemplateHelpers::RPIS<UVs...>> RenderPassInstance;

        static const blaU32 ms_renderPassId = renderPassId;
        static const size_t ms_VACount = sizeof...(VAs);
        static const size_t ms_UVCount = sizeof...(UVs);
    };

    template<int i, class RenderPass>
    class _GetRenderPassVADescriptorsInternal
    {
    public:
        static void Get(blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*>& typeDescriptors)
        {
            _GetRenderPassVADescriptorsInternal<i - 1, RenderPass>::Get(typeDescriptors);
            BLAInspectableVariables::ExposedVarTypeDescriptor* d =
                BLAInspectableVariables::TypeResolver<typename _RenderPassTemplateHelpers::InferVAType<i, typename RenderPass::RenderPassInstance::InstanceVertexAttributes>::VAType>::GetDescriptor();

            typeDescriptors.push_back(d);
        }
    };

    template<class RenderPass>
    class _GetRenderPassVADescriptorsInternal<0, RenderPass>
    {
    public:
        static void Get(blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*>& typeDescriptors)
        {
            BLAInspectableVariables::ExposedVarTypeDescriptor* d =
                BLAInspectableVariables::TypeResolver<typename _RenderPassTemplateHelpers::InferVAType<0, typename RenderPass::RenderPassInstance::InstanceVertexAttributes>::VAType>::GetDescriptor();

            typeDescriptors.push_back(d);
        }
    };

    template<class RenderPass>
    blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*> GetRenderPassVADescriptors()
    {
        blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*> typeDescriptors;

        _GetRenderPassVADescriptorsInternal<RenderPass::ms_VACount - 1, RenderPass>::Get(typeDescriptors);

        return typeDescriptors;
    }

    template<int i, class RenderPass>
    class _GetRenderPassUVDescriptorsInternal
    {
    public:
        static void Get(blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*>& typeDescriptors)
        {
            _GetRenderPassVADescriptorsInternal<i - 1, RenderPass>::Get(typeDescriptors);
            BLAInspectableVariables::ExposedVarTypeDescriptor* d =
                BLAInspectableVariables::TypeResolver<typename _RenderPassTemplateHelpers::InferVAType<i, typename RenderPass::RenderPassInstance::InstanceVertexAttributes>::VAType>::GetDescriptor();

            typeDescriptors.push_back(d);
        }
    };

    template<class RenderPass>
    class _GetRenderPassUVDescriptorsInternal<0, RenderPass>
    {
    public:
        static void Get(blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*>& typeDescriptors)
        {
            BLAInspectableVariables::ExposedVarTypeDescriptor* d =
                BLAInspectableVariables::TypeResolver<typename _RenderPassTemplateHelpers::InferVAType<0, typename RenderPass::RenderPassInstance::InstanceVertexAttributes>::VAType>::GetDescriptor();

            typeDescriptors.push_back(d);
        }
    };

    template<class RenderPass>
    blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*> GetRenderPassUVDescriptors()
    {
        blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*> typeDescriptors;

        _GetRenderPassUVDescriptorsInternal<RenderPass::ms_UVCount - 1, RenderPass>::Get(typeDescriptors);

        return typeDescriptors;
    }

    // This may only be needed for editor purposes... **May**
    class RenderPassRegistry
    {
        BLA_DECLARE_EXPORTED_SINGLETON(RenderPassRegistry);

        struct RenderPassRegistryEntry
        {
            blaU32 m_attachmentCount;
            blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*> m_vertexAttributesDescriptors;
            blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*> m_uniformValuesDescriptors;
        };

        typedef blaMap<blaU32, RenderPassRegistryEntry> RenderPassRegistryStorage;
        RenderPassRegistryStorage m_registry;

    public:

        void __RegisterRenderPass(blaU32 id,
            blaU32 attachmentCount,
            blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*>& vertexAttributesDescriptors,
            blaVector<BLAInspectableVariables::ExposedVarTypeDescriptor*>& uniformValuesDescriptor);
    };
}