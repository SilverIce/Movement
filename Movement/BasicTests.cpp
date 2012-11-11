#include "framework/typedefs_p.h"
#include "framework/gtest.h"
#include "framework/RdtscTimer.h"
#include "MSTime.h"
#include "LinkedList.h"
#include "ObjectGuid.h"

#include "MoveEnv.UnitTest.hpp"
#include "LinkedListSimple.h"

namespace Movement
{
    static_assert(true, "");
    static_assert(sizeof(uint8) == 1, "");
    static_assert(sizeof(int8) == 1, "");
    static_assert(sizeof(uint16) == 2, "");
    static_assert(sizeof(int16) == 2, "");
    static_assert(sizeof(uint32) == 4, "");
    static_assert(sizeof(int32) == 4, "");
    static_assert(sizeof(uint64) == 8, "");
    static_assert(sizeof(int64) == 8, "");

    TEST(MSTimeTest, testOperators) {
        const MSTime t2(2), t4(4);
        //const MSTime t2(4), t4(2);

        EXPECT_EQ(MSTime(2), (t4 - t2));

        EXPECT_EQ(MSTime(6), (t4 + t2));

        EXPECT_TRUE( t4 > t2 && t4 >= t2 );
        EXPECT_TRUE( t2 < t4 && t2 <= t4 );
        EXPECT_TRUE( t2 >= t2 && t2 <= t2 );
        EXPECT_TRUE( t2 != t4 );
        EXPECT_TRUE( t2 == t2 );
        EXPECT_EQ( (t2 + MSTime(2)), t4 );
        EXPECT_EQ( MSTime(2 + (0xFFFFFFFF - 4)), (t2 - t4) );
    }

    TEST(LinkedList, emptyState)
    {
        LinkedList<int> list;
        EXPECT_TRUE( list.empty() );
        EXPECT_TRUE( list.size() == 0 );
    }

    TEST(LinkedListElement, notLinked)
    {
        LinkedListElement<int> node;
        EXPECT_TRUE( !node.linked() );
        EXPECT_TRUE( node.Next() == NULL );
        EXPECT_TRUE( node.Next() == node.Previous() );
    }

    TEST(LinkedList, filled)
    {
        const int values[] = {0,1,2,20,4,5,6,7,8,12};

        LinkedListElement<int> node[CountOf(values)];
        LinkedList<int> list;
        for (int i = 0; i < CountOf(node); ++i) {
            node[i].Value = values[i];
            list.link_last(node[i]);
            EXPECT_TRUE( node[i].linked() );
        }

        EXPECT_TRUE( !list.empty() );
        EXPECT_TRUE( list.size() == CountOf(node) );

        {
            LinkedListElement<int> * Node = list.first();
            for (int i = 0; i < CountOf(node); ++i, Node = Node->Next()) {
                EXPECT_TRUE( Node != NULL  );
                EXPECT_TRUE( Node == &node[i] );
                EXPECT_TRUE( Node->Value == values[i]);
            }
        }
        {
            LinkedListElement<int> * Node = list.last();
            for (int i = (CountOf(node)-1); i >= 0; --i, Node = Node->Previous()) {
                EXPECT_TRUE( Node != NULL );
                EXPECT_TRUE( Node == &node[i] );
                EXPECT_TRUE( Node->Value == values[i]);
            }
        }

        {
            struct Visitor1 {
                int m_index;
                const int * m_values;
                testing::State& testState;
                Visitor1(const int * values, testing::State& state)
                    : m_index(0), m_values(values), testState(state) {}
                void operator()(const int & val) {
                    EXPECT_TRUE(val == m_values[m_index]);
                    ++m_index;
                }
            } visitor(values,testState);
            list.Visit(visitor);
            EXPECT_TRUE(visitor.m_index == CountOf(values) );
        }

        while (LinkedListElement<int> * Node = list.last()){
            list.delink( *Node );
            EXPECT_TRUE( !Node->linked() );
            Node = Node->Previous();
        }

        EXPECT_TRUE( list.size() == 0 );
        EXPECT_TRUE( list.empty() );
    }

    template<class T, int N>
    class StatArray
    {
        T _values[N];

    public:
        enum {
            Count = N,
        };
        inline T& operator [] (int idx) {
            assert_state(idx < N);
            return _values[idx];
        }
        inline const T& operator [] (int idx) const {
            assert_state(idx < N);
            return _values[idx];
        }
    };

    template<class T, int N>
    inline static bool CompareSeq(const T(&values)[N], const LinkedListSimple<T>& list)
    {
        const LinkedListElementSimple<T> * node = list.first();
        int i = 0;
        while(node && (i < N) && node->Value == values[i]) {
            ++i;
            node = node->Next();
        }
        return (i == N);
    }

    TEST(LinkedListSimple, all)
    {
        {
            LinkedListSimple<int> list;
            EXPECT_TRUE( list.empty() );
            EXPECT_TRUE( list.first() == nullptr );
            EXPECT_TRUE( list.count() == 0 );
        }

        const int values[] = {0,1,2,20,4,5,6,7,8,12};
        StatArray<LinkedListElementSimple<int>,CountOf(values)> nodes;
        for (int i = 0; i < CountOf(nodes); ++i) {
            EXPECT_TRUE( !nodes[i].linked() );
            nodes[i].Value = values[i];
        }

        {
            LinkedListSimple<int> list;
            list.link_first(nodes[0]);
            EXPECT_TRUE( !list.empty() );
            EXPECT_TRUE( list.first() == &nodes[0] );
            EXPECT_TRUE( list.count() == 1 );

            list.link_first(nodes[1]);
            int val[] = {1,0};
            EXPECT_TRUE( CompareSeq(val,list) );
            EXPECT_TRUE( list.first() == &nodes[1] );

            list.link_after(*list.first(), nodes[2]);
            int val2[] = {1,2,0};
            EXPECT_TRUE( CompareSeq(val2,list) );

            list.link_before(*list.first(), nodes[3]);
            int val3[] = {20,1,2,0};
            EXPECT_TRUE( CompareSeq(val3,list) );
            EXPECT_TRUE( list.count() == CountOf(val3) );

            list.delink_first();
            int val4[] = {1,2,0};
            EXPECT_TRUE( CompareSeq(val4,list) );
            EXPECT_TRUE( list.count() == CountOf(val4) );

            list.clear();
            EXPECT_TRUE( list.empty() );
            EXPECT_TRUE( list.first() == nullptr );
            EXPECT_TRUE( list.count() == 0 );
        }
    }

    TEST(ObjectGuid, basic)
    {
        const ObjectGuid guidIn((uint64(0xc0ca) << 32) | uint64(0xc01a));
        {
            ByteBuffer buffer;
            buffer << guidIn;
            ObjectGuid guidOut;
            buffer >> guidOut;
            EXPECT_EQ(guidIn, guidOut);
        }
        {
            const PackedGuid packed(guidIn);
            ObjectGuid guidOut(packed.Get());
            EXPECT_EQ(guidIn, guidOut);

            const uint8 maskPart = 3 | (3 << 4);
            EXPECT_TRUE( maskPart == packed.mask() );
            const uint32 packedPart = 0xc0cac01a;
            EXPECT_TRUE( memcmp(&packedPart, packed.packed(), 4) == 0 );
        }
        {
            ByteBuffer buffer;
            buffer << guidIn.WriteAsPacked();
            ObjectGuid guidOut;
            buffer >> guidOut.ReadAsPacked();
            EXPECT_EQ(guidIn, guidOut);
        }
    }

    TEST(ObjectGuid, performance)
    {
        ObjectGuid guidIn( (uint64)rand() | 
            ((uint64)rand() << 14) |
            ((uint64)rand() << 28) |
            ((uint64)rand() << 42) 
            );

        RdtscTimer pack, unpack;
        PackedGuid packed;
        ObjectGuid guidOut;
        int i = 1000;
        while(i-- > 0)
        {
            {
                RdtscCall c(pack);
                packed.Set(guidIn.GetRawValue());
            }
            {
                RdtscCall c(unpack);
                guidOut.SetRawValue(packed.Get());
            }
            // all these strange moves are done to not make compiler over-optimize code and remove packing, unpacking calls
            guidIn = guidOut;
        }
        log_console("guid packing   takes %u", pack.avg());
        log_console("guid unpacking takes %u", unpack.avg());
    }
}
