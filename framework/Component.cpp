#include "Component.h"
#include "gtest.h"
#include "RdtscTimer.h"

#include "typedefs_p.h"
#include <algorithm>
#include <vector>

namespace Movement
{
    static_assert(sizeof(AspectTypeId) == sizeof(void*), "");

    struct Type {
        AspectTypeId typeId;
        void * pointer;
        bool operator < (const Type& other) const {
            return typeId < other.typeId;
        }
    };

    class ComponentTree
    {
    private:
        std::vector<Type> m_types;
        int m_refCount;
    public:

        void addRef() { ++m_refCount;}

        bool release() {
            --m_refCount;
            return m_refCount <= 0;
        }

        int Count() const {
            return m_types.size();
        }

        void addAspect(AspectTypeId objectTypeId, void * object)
        {
            Type type = {objectTypeId, object};
            std::vector<Type>::const_iterator itr = std::lower_bound(m_types.begin(),m_types.end(),type);
            if (itr != m_types.end() && itr->typeId == type.typeId)
                throw std::runtime_error("aspect of same type added already");
            m_types.insert(itr, type);
        }

        void removeAspect(AspectTypeId objectTypeId)
        {
            Type fake = {objectTypeId, NULL};
            std::vector<Type>::const_iterator itr = std::lower_bound(m_types.begin(),m_types.end(),fake);
            if (itr != m_types.end() && itr->typeId == fake.typeId)
                m_types.erase(itr);
        }

        void* getAspect(AspectTypeId objectTypeId) const
        {
            Type fake = {objectTypeId, NULL};
            std::vector<Type>::const_iterator itr = std::lower_bound(m_types.begin(),m_types.end(),fake);
            if (itr != m_types.end() && itr->typeId == fake.typeId)
                return itr->pointer;
            else
                return nullptr;
        }
    };

    void Component::ComponentDetach() {
        if (m_tree) {
            m_tree->removeAspect(m_typeId);
            if (m_tree->release())
                delete m_tree;
            m_tree = nullptr;
        }
    }

    Component::~Component()
    {
        ComponentDetach();
    }

    void Component::_ComponentInit(void * me, AspectTypeId objectTypeId, ComponentTree * tree)
    {
        assert_state(!m_tree);
        assert_state(me);

        if (!tree)
            tree = new ComponentTree();
        m_tree = tree;
        m_typeId = objectTypeId;
        m_tree->addRef();
        m_tree->addAspect(objectTypeId, me);
    }

    void* Component::_getAspect(AspectTypeId objectTypeId) const {
        assert_state(m_tree);
        return m_tree->getAspect(objectTypeId);
    }

}

namespace Movement
{
    struct TypeA : ComponentT<TypeA> {
        int a;
    };
    struct TypeB : ComponentT<TypeB> {
        int b;
    };
    struct TypeC : ComponentT<TypeC> {
        int c;
    };

    TEST(TypeContainer, typeId)
    {
        // expects that different object types has different ids
        EXPECT_TRUE( TypeA::getTypeId() != TypeB::getTypeId() );


    }

    TEST(TypeContainer, ComponentTree)
    {
        ComponentTree cnt;
        EXPECT_TRUE( cnt.Count() == 0 );

        TypeA atype;
        cnt.addAspect(atype.getTypeId(), &atype);

        EXPECT_TRUE( cnt.getAspect(atype.getTypeId()) == &atype );
        EXPECT_TRUE( cnt.Count() == 1 );

        TypeB btype;
        cnt.addAspect(btype.getTypeId(), &btype);
        EXPECT_TRUE( cnt.getAspect(btype.getTypeId()) == &btype );
        EXPECT_TRUE( cnt.Count() == 2 );

        TypeA atypeAnother;
        EXPECT_THROW( cnt.addAspect(atypeAnother.getTypeId(),&atypeAnother), std::runtime_error );
        EXPECT_TRUE( cnt.Count() == 2 );

        EXPECT_TRUE( cnt.getAspect(TypeC::getTypeId()) == NULL );
        EXPECT_TRUE( cnt.Count() == 2 );

        cnt.removeAspect(TypeB::getTypeId());
        EXPECT_TRUE( cnt.getAspect(TypeB::getTypeId()) == NULL );
        EXPECT_TRUE( cnt.Count() == 1 );

        cnt.removeAspect(TypeA::getTypeId());
        EXPECT_TRUE( cnt.getAspect(TypeA::getTypeId()) == NULL );
        EXPECT_TRUE( cnt.Count() == 0 );
    }

    TEST(TypeContainer, Component)
    {
        TypeA atype;
        atype.ComponentInit(&atype);

        EXPECT_TRUE( atype.as<TypeA>() == &atype );
        EXPECT_TRUE( atype.as<TypeB>() == nullptr );
   
        TypeB btype;
        btype.ComponentInit(&btype, atype);

        EXPECT_TRUE( atype.as<TypeA>() == &atype );
        EXPECT_TRUE( btype.as<TypeB>() == &btype );
    }
}
