namespace Tasks { namespace detail
{
    TEST(POD_ArrayTest, vec_test_1)
    {
      POD_Array<int> v1; // Empty vector of integers.

      EXPECT_TRUE( v1.empty() == true );
      EXPECT_TRUE( v1.size() == 0 );

      // EXPECT_TRUE( v1.max_size() == INT_MAX / sizeof(int) );
      // cout << "max_size = " << v1.max_size() << endl;
      v1.push_back(42); // Add an integer to the vector.

      EXPECT_TRUE( v1.size() == 1 );

      EXPECT_TRUE( v1[0] == 42 );
    }

    TEST(POD_ArrayTest, vec_test_2)
    {
      POD_Array<double> v1; // Empty vector of doubles.
      v1.push_back(32.1);
      v1.push_back(40.5);
      POD_Array<double> v2; // Another empty vector of doubles.
      v2.push_back(3.56);

      EXPECT_TRUE( v1.size() == 2 );
      EXPECT_TRUE( v1[0] == 32.1 );
      EXPECT_TRUE( v1[1] == 40.5 );

      EXPECT_TRUE( v2.size() == 1 );
      EXPECT_TRUE( v2[0] == 3.56 );
      size_t v1Cap = v1.capacity();
      size_t v2Cap = v2.capacity();

      v1.swap(v2); // Swap the vector's contents.

      EXPECT_TRUE( v1.size() == 1 );
      EXPECT_TRUE( v1.capacity() == v2Cap );
      EXPECT_TRUE( v1[0] == 3.56 );

      EXPECT_TRUE( v2.size() == 2 );
      EXPECT_TRUE( v2.capacity() == v1Cap );
      EXPECT_TRUE( v2[0] == 32.1 );
      EXPECT_TRUE( v2[1] == 40.5 );

      v2 = v1; // Assign one vector to another.

      EXPECT_TRUE( v2.size() == 1 );
      EXPECT_TRUE( v2[0] == 3.56 );
    }

    TEST(POD_ArrayTest, vec_test_3)
    {
      typedef POD_Array<char> vec_type;

      vec_type v1; // Empty vector of characters.
      v1.push_back('h');
      v1.push_back('i');

      EXPECT_TRUE( v1.size() == 2 );
      EXPECT_TRUE( v1[0] == 'h' );
      EXPECT_TRUE( v1[1] == 'i' );

      vec_type v2(v1.begin(), v1.end());
      v2[1] = 'o'; // Replace second character.

      EXPECT_TRUE( v2.size() == 2 );
      EXPECT_TRUE( v2[0] == 'h' );
      EXPECT_TRUE( v2[1] == 'o' );

      /*EXPECT_TRUE( (v1 == v2) == false );

      EXPECT_TRUE( (v1 < v2) == true );*/
    }

    TEST(POD_ArrayTest, vec_test_4)
    {
      POD_Array<int> v(4);

      v[0] = 1;
      v[1] = 4;
      v[2] = 9;
      v[3] = 16;

      EXPECT_EQ( 1, v.front() );
      EXPECT_EQ( 16, v.back() );

      v.push_back(25);

      EXPECT_EQ( 25, v.back() );
      EXPECT_EQ( 5, v.size() );

      v.pop_back();

      EXPECT_EQ( 16, v.back() );
      EXPECT_EQ( 4, v.size() );
    }

    TEST(POD_ArrayTest, vec_test_5)
    {
      int array [] = { 1, 4, 9, 16 };

      POD_Array<int> v(array, array + 4);

      EXPECT_TRUE( v.size() == 4 );

      EXPECT_TRUE( v[0] == 1 );
      EXPECT_TRUE( v[1] == 4 );
      EXPECT_TRUE( v[2] == 9 );
      EXPECT_TRUE( v[3] == 16 );
    }

    TEST(POD_ArrayTest, vec_test_6)
    {
      int array [] = { 1, 4, 9, 16, 25, 36 };

      POD_Array<int> v(array, array + 6);
      POD_Array<int>::iterator vit;

      EXPECT_TRUE( v.size() == 6 );
      EXPECT_TRUE( v[0] == 1 );
      EXPECT_TRUE( v[1] == 4 );
      EXPECT_TRUE( v[2] == 9 );
      EXPECT_TRUE( v[3] == 16 );
      EXPECT_TRUE( v[4] == 25 );
      EXPECT_TRUE( v[5] == 36 );

      vit = v.erase( v.begin() ); // Erase first element.
      EXPECT_TRUE( *vit == 4 );

      EXPECT_TRUE( v.size() == 5 );
      EXPECT_TRUE( v[0] == 4 );
      EXPECT_TRUE( v[1] == 9 );
      EXPECT_TRUE( v[2] == 16 );
      EXPECT_TRUE( v[3] == 25 );
      EXPECT_TRUE( v[4] == 36 );

      vit = v.erase(v.end() - 1); // Erase last element.
      EXPECT_TRUE( vit == v.end() );

      EXPECT_TRUE( v.size() == 4 );
      EXPECT_TRUE( v[0] == 4 );
      EXPECT_TRUE( v[1] == 9 );
      EXPECT_TRUE( v[2] == 16 );
      EXPECT_TRUE( v[3] == 25 );


      v.erase(v.begin() + 1, v.end() - 1); // Erase all but first and last.

      EXPECT_EQ( 2, v.size() );
      EXPECT_EQ( 4, v[0] );
      EXPECT_EQ( 25, v[1] );
    }

    TEST(POD_ArrayTest, vec_test_7)
    {
      int array1 [] = { 1, 4, 25 };
      int array2 [] = { 9, 16 };

      POD_Array<int> v(array1, array1 + 3);
      POD_Array<int>::iterator vit;
      vit = v.insert(v.begin(), 0); // Insert before first element.
      EXPECT_TRUE( *vit == 0 );

      vit = v.insert(v.end(), 36);  // Insert after last element.
      EXPECT_TRUE( *vit == 36 );

      EXPECT_TRUE( v.size() == 5 );
      EXPECT_TRUE( v[0] == 0 );
      EXPECT_TRUE( v[1] == 1 );
      EXPECT_TRUE( v[2] == 4 );
      EXPECT_TRUE( v[3] == 25 );
      EXPECT_TRUE( v[4] == 36 );

      // Insert contents of array2 before fourth element.
      /*v.insert(v.begin() + 3, array2, array2 + 2);

      EXPECT_TRUE( v.size() == 7 );

      EXPECT_TRUE( v[0] == 0 );
      EXPECT_TRUE( v[1] == 1 );
      EXPECT_TRUE( v[2] == 4 );
      EXPECT_TRUE( v[3] == 9 );
      EXPECT_TRUE( v[4] == 16 );
      EXPECT_TRUE( v[5] == 25 );
      EXPECT_TRUE( v[6] == 36 );*/

      v.clear();
      EXPECT_TRUE( v.empty() );

      /*v.insert(v.begin(), 5, 10);
      EXPECT_TRUE( v.size() == 5 );
      EXPECT_TRUE( v[0] == 10 );
      EXPECT_TRUE( v[1] == 10 );
      EXPECT_TRUE( v[2] == 10 );
      EXPECT_TRUE( v[3] == 10 );
      EXPECT_TRUE( v[4] == 10 );*/

      /*
      {
        POD_Array<float> vf(2.0f, 3.0f);
        EXPECT_TRUE( vf.size() == 2 );
        EXPECT_TRUE( vf.front() == 3.0f );
        EXPECT_TRUE( vf.back() == 3.0f );
      }
      */
    }

    struct TestStruct
    {
      unsigned int a[3];
    };

    TEST(POD_ArrayTest, capacity)
    {
      {
        POD_Array<int> v;

        EXPECT_TRUE( v.capacity() == 0 );
        v.push_back(42);
        EXPECT_TRUE( v.capacity() >= 1 );
        v.reserve(5000);
        EXPECT_TRUE( v.capacity() >= 5000 );
      }

      {
        //Test that used to generate an assertion when using __debug_alloc.
        POD_Array<TestStruct> va;
        va.reserve(1);
        va.reserve(2);
      }
    }

    TEST(POD_ArrayTest, at) {
      POD_Array<int> v;
      POD_Array<int> const& cv = v;

      v.push_back(10);
      EXPECT_TRUE( v.at(0) == 10 );
      v.at(0) = 20;
      EXPECT_TRUE( cv.at(0) == 20 );

    /*#if !defined (STLPORT) || defined (_STLP_USE_EXCEPTIONS)
      try {
        v.at(1) = 20;
        CPPUNIT_FAIL;
      }
      catch (out_of_range const&) {
      }
      catch (...) {
        CPPUNIT_FAIL;
      }
    #endif*/
    }

    TEST(POD_ArrayTest, pointer)
    {
      POD_Array<int *> v1;
      POD_Array<int *> v2 = v1;
      POD_Array<int *> v3;

      v3.insert( v3.end(), v1.begin(), v1.end() );
    }

    TEST(POD_ArrayTest, auto_ref)
    {
      POD_Array<int> ref;
      for (int i = 0; i < 5; ++i) {
        ref.push_back(i);
      }

      POD_Array<POD_Array<int> > v_v_int(1, ref);
      v_v_int.push_back(v_v_int[0]);
      v_v_int.push_back(ref);
      v_v_int.push_back(v_v_int[0]);
      v_v_int.push_back(v_v_int[0]);
      v_v_int.push_back(ref);

      /*POD_Array<POD_Array<int> >::iterator vvit(v_v_int.begin()), vvitEnd(v_v_int.end());
      for (; vvit != vvitEnd; ++vvit) {
        EXPECT_TRUE( *vvit == ref );
      }*/

      /*
       * Forbidden by the Standard:
      v_v_int.insert(v_v_int.end(), v_v_int.begin(), v_v_int.end());
      for (vvit = v_v_int.begin(), vvitEnd = v_v_int.end();
           vvit != vvitEnd; ++vvit) {
        EXPECT_TRUE( *vvit == ref );
      }
       */
    }

    struct Point {
      int x, y;
    };

    struct PointEx : public Point {
      PointEx() : builtFromBase(false) {}
      PointEx(const Point&) : builtFromBase(true) {}

      bool builtFromBase;
    };

   /* #if defined (STLPORT)
    #  if defined (_STLP_USE_NAMESPACES)
    namespace std {
    #  endif
      _STLP_TEMPLATE_NULL
      struct __type_traits<PointEx> {
        typedef __false_type has_trivial_default_constructor;
        typedef __true_type has_trivial_copy_constructor;
        typedef __true_type has_trivial_assignment_operator;
        typedef __true_type has_trivial_destructor;
        typedef __true_type is_POD_type;
      };
    #  if defined (_STLP_USE_NAMESPACES)
    }
    #  endif
    #endif

    //This test check that vector implementation do not over optimize
    //operation as PointEx copy constructor is trivial
    TEST(POD_ArrayTest, optimizations_check)
    {
    #if !defined (STLPORT) || !defined (_STLP_NO_MEMBER_TEMPLATES)
      POD_Array<Point> v1(1);
      EXPECT_TRUE( v1.size() == 1 );

      POD_Array<PointEx> v2(v1.begin(), v1.end());
      EXPECT_TRUE( v2.size() == 1 );
      EXPECT_TRUE( v2[0].builtFromBase == true );
    #endif
    }
*/

    TEST(POD_ArrayTest, assign_check)
    {
    #if !defined (STLPORT) || !defined (_STLP_NO_MEMBER_TEMPLATES)
      POD_Array<int> v(3,1);
      int array[] = { 1, 2, 3, 4, 5 };
      
      v.assign( array, array + 5 );
      EXPECT_TRUE( v[4] == 5 );
      EXPECT_TRUE( v[0] == 1 );
      EXPECT_TRUE( v[1] == 2 );
    #endif
    }

    TEST(POD_ArrayTest, iterators)
    {
      POD_Array<int> vint(10, 0);
      POD_Array<int> const& crvint = vint;

      EXPECT_TRUE( vint.begin() == vint.begin() );
      EXPECT_TRUE( crvint.begin() == vint.begin() );
      EXPECT_TRUE( vint.begin() == crvint.begin() );
      EXPECT_TRUE( crvint.begin() == crvint.begin() );

      EXPECT_TRUE( vint.begin() != vint.end() );
      EXPECT_TRUE( crvint.begin() != vint.end() );
      EXPECT_TRUE( vint.begin() != crvint.end() );
      EXPECT_TRUE( crvint.begin() != crvint.end() );

      EXPECT_TRUE( vint.rbegin() == vint.rbegin() );
      // Not Standard:
      //EXPECT_TRUE( vint.rbegin() == crvint.rbegin() );
      //EXPECT_TRUE( crvint.rbegin() == vint.rbegin() );
      EXPECT_TRUE( crvint.rbegin() == crvint.rbegin() );

      EXPECT_TRUE( vint.rbegin() != vint.rend() );
      // Not Standard:
      //EXPECT_TRUE( vint.rbegin() != crvint.rend() );
      //EXPECT_TRUE( crvint.rbegin() != vint.rend() );
      EXPECT_TRUE( crvint.rbegin() != crvint.rend() );
    }


    /*#if !defined (STLPORT) || \
        !defined (_STLP_USE_PTR_SPECIALIZATIONS) || defined (_STLP_CLASS_PARTIAL_SPECIALIZATION)
    / * Simple compilation test: Check that nested types like iterator
     * can be access even if type used to instanciate container is not
     * yet completely defined.
     * /
    class IncompleteClass
    {
      POD_Array<IncompleteClass> instances;
      typedef POD_Array<IncompleteClass>::iterator it;
    };
    #endif*/

    /*#if defined (STLPORT)
    #  define NOTHROW _STLP_NOTHROW
    #else
    #  define NOTHROW throw()
    #endif

    / * This allocator implementation purpose is simply to break some
     * internal STLport mecanism specific to the STLport own allocator
     * implementation. * /
    template <class _Tp>
    struct NotSTLportAllocator : public allocator<_Tp> {
    #if !defined (STLPORT) || defined (_STLP_MEMBER_TEMPLATE_CLASSES)
      template <class _Tp1> struct rebind {
        typedef NotSTLportAllocator<_Tp1> other;
      };
    #endif
      NotSTLportAllocator() NOTHROW {}
    #if !defined (STLPORT) || defined (_STLP_MEMBER_TEMPLATES)
      template <class _Tp1> NotSTLportAllocator(const NotSTLportAllocator<_Tp1>&) NOTHROW {}
    #endif
      NotSTLportAllocator(const NotSTLportAllocator<_Tp>&) NOTHROW {}
      ~NotSTLportAllocator() NOTHROW {}
    };*/

    /* This test check a potential issue with empty base class
     * optimization. Some compilers (VC6) do not implement it
     * correctly resulting ina wrong behavior. */
    /*TEST(POD_ArrayTest, ebo()
    {
      // We use heap memory as test failure can corrupt vector internal
      // representation making executable crash on vector destructor invocation.
      // We prefer a simple memory leak, internal corruption should be reveal
      // by size or capacity checks.
      typedef POD_Array<int, NotSTLportAllocator<int> > V;
      V *pv1 = new V(1, 1);
      V *pv2 = new V(10, 2);

      size_t v1Capacity = pv1->capacity();
      size_t v2Capacity = pv2->capacity();

      pv1->swap(*pv2);

      EXPECT_TRUE( pv1->size() == 10 );
      EXPECT_TRUE( pv1->capacity() == v2Capacity );
      EXPECT_TRUE( (*pv1)[5] == 2 );

      EXPECT_TRUE( pv2->size() == 1 );
      EXPECT_TRUE( pv2->capacity() == v1Capacity );
      EXPECT_TRUE( (*pv2)[0] == 1 );

      delete pv2;
      delete pv1;
    }*/
}
}
