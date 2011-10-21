namespace Movement
{
    extern void MoveSplineTests();

    void testUnitMoveFlag()
    {
        UnitMoveFlag::eUnitMoveFlags Flag = UnitMoveFlag::AllowSwimFlyTransition;
        UnitMoveFlag f(Flag);
        check( f.hasFlag(Flag) );
        check( (f & Flag) != 0 );
        check( f.raw == Flag );
        check( f.allowSwimFlyTransition );
        check( f.ToString().find("AllowSwimFlyTransition") != std::string::npos );
        check( sizeof(f.raw) == sizeof(UnitMoveFlag) );
    }

    void UnitMovementTests()
    {
        MoveSplineTests();
        testUnitMoveFlag();
    }
}
