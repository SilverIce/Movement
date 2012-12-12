#include "Component.h"
#include "gtest.h"
#include "RdtscTimer.h"

#include "typedefs_p.h"
#include <QtCore/QVector>
#include <QtCore/QTextStream>
#include <QtCore/QAtomicInt>
#include <typeinfo>
#include <stdexcept>

namespace Movement
{
    static_assert(sizeof(AspectTypeId) == sizeof(void*), "");

    struct Type {
        AspectTypeId typeId;
        Component * pointer;
        bool operator < (const Type& other) const {
            return typeId < other.typeId;
        }
        bool operator == (const Type& other) const {
            return typeId == other.typeId;
        }
        bool operator != (const Type& other) const {
            return typeId != other.typeId;
        }
    };
    Q_DECLARE_TYPEINFO(Type, Q_PRIMITIVE_TYPE|Q_MOVABLE_TYPE);

    class ComponentTree
    {
    private:
        QVector<Type> m_types;
        QAtomicInt m_refCount;

        inline QVector<Type>::iterator find(const Type& type) {
            return qLowerBound(m_types.begin(), m_types.end(), type);
        }

    public:

        explicit ComponentTree() {}

        ~ComponentTree() {
            assert_state(m_refCount <= 0);
            assert_state(m_types.empty());
        }

        void addRef() { m_refCount.ref();}

        bool release() {
            return !m_refCount.deref();
        }

        int32 refCount() const {
            return m_refCount;
        }

        int32 Count() const {
            return m_types.size();
        }

        const Component& operator[] (int32 idx) const {
            return *m_types[idx].pointer;
        }

        void addAspect(AspectTypeId objectTypeId, Component & object)
        {
            Type type = {objectTypeId, &object};
            QVector<Type>::iterator itr = find(type);
            assert_or_throw_msg(itr == m_types.end() || itr->typeId != type.typeId,
                 "aspect of same type added already", Exception<Component>);
            m_types.insert(itr, type);
        }

        void removeAspect(AspectTypeId objectTypeId)
        {
            Type fake = {objectTypeId, NULL};
            QVector<Type>::iterator itr = find(fake);
            if (itr != m_types.end() && itr->typeId == fake.typeId)
                m_types.erase(itr);
        }

        Component* getAspect(AspectTypeId objectTypeId) const
        {
            Type fake = {objectTypeId, NULL};
            QVector<Type>::const_iterator itr = qLowerBound(m_types, fake);
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
        // code below should NOT throw!
        ComponentDetach();
    }

    void Component::_ComponentInit(Component* me, AspectTypeId objectTypeId, ComponentTree * tree)
    {
        assert_state(!m_tree);
        assert_state(this == me);

        if (!tree)
            tree = new ComponentTree();
        m_tree = tree;
        m_typeId = objectTypeId;
        m_tree->addRef();
        m_tree->addAspect(objectTypeId, *this);
    }

    void Component::_ComponentAttach(AspectTypeId objectTypeId, Component * component)
    {
        assert_state(component);
        assert_state(m_tree);
        component->_ComponentInit(component, objectTypeId, m_tree);
    }

    Component* Component::_getAspect(AspectTypeId objectTypeId) const {
        assert_state(m_tree);
        if (m_typeId == objectTypeId)
            return const_cast<Component*>(this); // uh.. why i have declared cast methods as constant?
        else
            return m_tree->getAspect(objectTypeId);
    }

    Component& Component::_as(AspectTypeId objectTypeId) const {
        Component * object = _getAspect(objectTypeId);
        assert_or_throw(object != nullptr, Exception<Component>);
        return *object;
    }

    void Component::toString(QTextStream& st) const
    {
        st << endl << "<no description>";
    }

    QString Component::toStringAll() const
    {
        QString string;
        QTextStream str(&string, QIODevice::WriteOnly);
        if (!m_tree) {
            str << endl << "Not attached component";
            str << endl << typeid(*this).name();
            toString(str);
        }
        else {
            str << endl << "Component amount " << m_tree->Count();
            for (int32 idx = 0; idx < m_tree->Count(); ++idx) {
                str << endl << typeid((*m_tree)[idx]).name() << " {";
                (*m_tree)[idx].toString(str);
                str << endl << "}";
            }
        }
        return *str.string();
    }
}

namespace Movement
{
    struct TypeA : Component {
        COMPONENT_TYPEID(TypeA);
        static AspectTypeId getTypeId() { return HasTypeId::getTypeId();}
        int a;
    };
    struct TypeB : Component {
        COMPONENT_TYPEID(TypeB);
        static AspectTypeId getTypeId() { return HasTypeId::getTypeId();}
        int b;
    };
    struct TypeC : Component {
        COMPONENT_TYPEID(TypeC);
        static AspectTypeId getTypeId() { return HasTypeId::getTypeId();}
        int c;
    };
    struct TypeD : Component {
        COMPONENT_TYPEID(TypeD);
        static AspectTypeId getTypeId() { return HasTypeId::getTypeId();}
        int d;
    };

    TEST(TypeContainer, typeId)
    {
        // expects that different object types have different ids
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
        EXPECT_THROW( cnt.addAspect(atypeAnother.getTypeId(),atypeAnother), Exception<Component> );
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

    TEST(TypeContainer, cast)
    {
        struct BaseObj {
            virtual ~BaseObj() {}
            BaseObj () {}
            char field1[100];
        };

        struct DerObj : BaseObj, Component {
            COMPONENT_TYPEID(DerObj);
            char field2[8];
        };

        DerObj type;
        type.ComponentInit(&type);

        TypeA atype;
        type.ComponentAttach(&atype);

        EXPECT_TRUE( atype.getAspect<DerObj>() == &type );
    }

    TEST(TypeContainer, Component)
    {
        TypeA atype;
        atype.ComponentInit(&atype);

        EXPECT_TRUE( atype.getAspect<TypeA>() == &atype );
        EXPECT_TRUE( atype.getAspect<TypeB>() == nullptr );
        EXPECT_THROW( atype.as<TypeB>(), Exception<Component> );
   
        TypeB btype;
        atype.ComponentAttach(&btype);

        EXPECT_TRUE( atype.getAspect<TypeA>() == &atype );
        EXPECT_TRUE( btype.getAspect<TypeB>() == &btype );
        EXPECT_TRUE( &atype.as<TypeB>() == &btype );

        btype.ComponentDetach();
        EXPECT_TRUE( atype.getAspect<TypeB>() == nullptr );

        {
            TypeC *ctype = new TypeC;
            atype.ComponentAttach(ctype);
            EXPECT_TRUE( atype.getAspect<TypeC>() == ctype );

            delete ctype;
            EXPECT_TRUE( atype.getAspect<TypeC>() == nullptr );
        }
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
                obj |= (size_t)atype.getAspect<TypeA>();
            }
            assert_state(tim.count() > callsCount);
            callsCount = tim.count();
            {
                RdtscCall c(tim);
                obj |= (size_t)atype.getAspect<TypeB>();
            }
            assert_state(tim.count() > callsCount);
            callsCount = tim.count();
            {
                RdtscCall c(tim);
                obj |= (size_t)atype.getAspect<TypeC>();
            }
            assert_state(tim.count() > callsCount);
            callsCount = tim.count();
            {
                RdtscCall c(tim);
                obj |= (size_t)atype.getAspect<TypeD>();
            }
        }
        log_console("average as<T> cast takes %u CPU ticks, casts count %u", tim.avg(), (uint32)tim.count());

        RdtscTimer tm;
        Component * com = &ctype;
        for (int i = 0; i < 8000; ++i)
        {
            tm.onCallBegin();
            com = dynamic_cast<TypeC*>(com);
            tm.onCallEnd();
            assert_state(com && com == &ctype);
        }
        log_console("average dynamic_cast<T> takes %u CPU ticks, casts count %u", tm.avg(), (uint32)tim.count());
    }
}
