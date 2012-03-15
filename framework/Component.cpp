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
        Component * pointer;
        bool operator < (const Type& other) const {
            return typeId < other.typeId;
        }
    };

    class ComponentTree
    {
    private:
        std::vector<Type> m_types;
        int32 m_refCount;
    public:

        explicit ComponentTree() : m_refCount(0) {}
        ~ComponentTree() { assert_state(m_refCount <= 0);}

        void addRef() { ++m_refCount;}

        bool release() {
            --m_refCount;
            return m_refCount <= 0;
        }

        int32 refCount() const {
            return m_refCount;
        }

        int32 Count() const {
            return m_types.size();
        }

        void addAspect(AspectTypeId objectTypeId, Component & object)
        {
            Type type = {objectTypeId, &object};
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

        Component* getAspect(AspectTypeId objectTypeId) const
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
        m_this = me;
        m_typeId = objectTypeId;
        m_tree->addRef();
        m_tree->addAspect(objectTypeId, *this);
    }

    void Component::_ComponentAttach(void * object, AspectTypeId objectTypeId, Component * com)
    {
        assert_state(com);
        assert_state(m_tree);
        com->_ComponentInit(object, objectTypeId, m_tree);
    }

    void* Component::_getAspect(AspectTypeId objectTypeId) const {
        assert_state(m_tree);
        if (Component * com = m_tree->getAspect(objectTypeId))
            return com->m_this;
        return nullptr;
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
    struct TypeD : ComponentT<TypeD> {
        int d;
    };

    TEST(TypeContainer, typeId)
    {
        // expects that different object types has different ids
        EXPECT_TRUE( TypeA::getTypeId() != TypeB::getTypeId() );
    }

    TEST(TypeContainer, ComponentTree_ref_count)
    {
        ComponentTree tree;
        EXPECT_TRUE( tree.refCount() == 0 );

        const int refsCount = 5;
        for (int idx = 0; idx < refsCount; ) {
            tree.addRef();
            ++idx;
            EXPECT_TRUE( tree.refCount() == idx );
        }
        for (int idx = refsCount; idx > 0; ) {
            if (tree.release())
                EXPECT_TRUE( tree.refCount() == 0 );
            --idx;
            EXPECT_TRUE( tree.refCount() == idx );
        }
    }

    TEST(TypeContainer, ComponentTree)
    {
        ComponentTree cnt;
        EXPECT_TRUE( cnt.Count() == 0 );

        TypeA atype;
        cnt.addAspect(atype.getTypeId(), atype);

        EXPECT_TRUE( cnt.getAspect(atype.getTypeId()) == &atype );
        EXPECT_TRUE( cnt.Count() == 1 );

        TypeB btype;
        cnt.addAspect(btype.getTypeId(), btype);
        EXPECT_TRUE( cnt.getAspect(btype.getTypeId()) == &btype );
        EXPECT_TRUE( cnt.Count() == 2 );

        TypeA atypeAnother;
        EXPECT_THROW( cnt.addAspect(atypeAnother.getTypeId(),atypeAnother), std::runtime_error );
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
        atype.ComponentAttach(&btype);

        EXPECT_TRUE( atype.as<TypeA>() == &atype );
        EXPECT_TRUE( btype.as<TypeB>() == &btype );
    }

    TEST(TypeContainer, as_cast_performance)
    {
        TypeA atype;
        TypeB btype;
        TypeC ctype;
        TypeD dtype;

        atype.ComponentInit(&atype);
        atype.ComponentAttach(&btype);
        atype.ComponentAttach(&ctype);
        atype.ComponentAttach(&dtype);

        RdtscTimer tim;
        size_t obj = 0;
        uint64 callsCount = 0;
        for (int idx = 0; idx < 2000; ++idx)
        {
            {
                RdtscCall c(tim);
                obj |= (size_t)atype.as<TypeA>();
            }
            assert_state(tim.count() > callsCount);
            callsCount = tim.count();
            {
                RdtscCall c(tim);
                obj |= (size_t)atype.as<TypeB>();
            }
            assert_state(tim.count() > callsCount);
            callsCount = tim.count();
            {
                RdtscCall c(tim);
                obj |= (size_t)atype.as<TypeC>();
            }
            assert_state(tim.count() > callsCount);
            callsCount = tim.count();
            {
                RdtscCall c(tim);
                obj |= (size_t)atype.as<TypeD>();
            }
        }
        log_console("average as<T> cast takes %u CPU ticks, casts count %u", tim.avg(), (uint32)tim.count());
    }
}
