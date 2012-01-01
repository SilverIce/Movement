#include "typedefs_p.h"
#include "MSTime.h"
#include "LinkedList.h"
#include "ObjectGuid.h"
#include "gtest/gtest.h"

#include "MoveEnv.UnitTest.hpp"
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
                EXPECT_TRUE( Node );
                EXPECT_TRUE( Node == &node[i] );
                EXPECT_TRUE( Node->Value == values[i]);
            }
        }
        {
            LinkedListElement<int> * Node = list.last();
            for (int i = (CountOf(node)-1); i >= 0; --i, Node = Node->Previous()) {
                EXPECT_TRUE( Node );
                EXPECT_TRUE( Node == &node[i] );
                EXPECT_TRUE( Node->Value == values[i]);
            }
        }

        {
            struct Visitor1 {
                int m_index;
                const int * m_values;
                Visitor1(const int * values) : m_index(0), m_values(values) {}
                void operator()(const int & val) {
                    EXPECT_TRUE(val == m_values[m_index]);
                    ++m_index;
                }
            } visitor(values);
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
            PackedGuid packed(guidIn);
            ObjectGuid guidOut(packed.Get());
            EXPECT_EQ(guidIn, guidOut);
        }
        {
            ByteBuffer buffer;
            buffer << guidIn.WriteAsPacked();
            ObjectGuid guidOut;
            buffer >> guidOut.ReadAsPacked();
            EXPECT_EQ(guidIn, guidOut);
        }
    }
}
