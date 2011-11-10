#include "typedefs_p.h"
#include "MSTime.h"
#include "LinkedList.h"
#include "gtest/gtest.h"

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
        EXPECT_TRUE( t2 != t4 );
        EXPECT_EQ( (t2 + MSTime(2)), t4 );
        EXPECT_EQ( MSTime(2 + (0xFFFFFFFF - 4)), (t2 - t4) );
    }

    TEST(LinkedList, basic)
    {
        LinkedListElement<int> node[10];
        LinkedList<int> list;

        EXPECT_TRUE(list.empty());
        for (int i = 0; i < CountOf(node); ++i)
            EXPECT_TRUE( !node[i].linked() );

        {
            for (int i = 0; i < CountOf(node); ++i) {
                node[i].Value = i;
                list.link_last(node[i]);
                EXPECT_TRUE( node[i].linked() );
            }

            EXPECT_TRUE( !list.empty() );

            LinkedListElement<int> * Node = list.first();
            int value = 0;
            while(Node){
                EXPECT_EQ( value, Node->Value );
                Node = Node->Next();
                ++value;
            }

            Node = list.last();
            value = 9;
            while(Node){
                EXPECT_EQ( value, Node->Value );
                Node = Node->Previous();
                --value;
            }

            while (Node = list.last()){
                list.delink( *Node );
                EXPECT_TRUE( !Node->linked() );
                Node = Node->Previous();
            }
        }
    }

    void RunTests()
    {
    }
}
