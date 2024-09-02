#include <gtest/gtest.h>
#include <conflicts/conflicts.hpp>

enum NiceGuys
{
	Kyle,
	John,
	Harry,
	Jack,
	Joe
};

class ConflictsTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		con1.add(Kyle, Harry);	// Kyle is in conflict with Harry
		con1.add(Harry, Joe);	// Harry is in conflict with Joe
		con1.add(Jack, Joe);	// Jack is in conflict with Joe
		con1.add(Kyle, Jack);	// Kyle is in conflict with John
		con2.add(Kyle, Harry);
		con2.add(Harry, Joe);
		con2.add(Jack, Joe);
		con2.add(John, Jack);	// John is in conflict with Kyle (cascading on)
	}

	// void TearDown() override {}

	Conflicts<NiceGuys> con0;
	Conflicts<NiceGuys> con1;
	Conflicts<NiceGuys> con2{ true };		// cascading on
};

using ConflictsDeathTest = ConflictsTest;

TEST_F(ConflictsTest, Initialization)
{
	EXPECT_TRUE(con0.empty());
	EXPECT_EQ(con0.size(), 0);
	EXPECT_FALSE(con1.empty());
	EXPECT_EQ(con1.size(), 4);
	EXPECT_FALSE(con1.cascading());
	EXPECT_TRUE(con2.cascading());
}

TEST_F(ConflictsDeathTest, Assertions)
{
	EXPECT_DEATH(con1.add(Joe, Joe), "");		// Cannot be in conflict with itself
	EXPECT_DEATH(con1.add(Joe, Harry), "");		// Already exists
	EXPECT_DEATH(con2.add(Kyle, John), "");		// This is not allowed while cascading is on (implicit)
}

TEST_F(ConflictsTest, In_Conflict)
{
	EXPECT_TRUE(con1.in_conflict(Joe));
	EXPECT_FALSE(con1.in_conflict(John));
	EXPECT_TRUE(con1.in_conflict(Harry, Kyle));
	EXPECT_FALSE(con1.in_conflict(Kyle, Joe));
	EXPECT_TRUE(con2.in_conflict(Kyle, John));		// True while cascading is on
}

TEST_F(ConflictsTest, All_Conflicts)
{
	auto cons_deep = con1.all_conflicts(Jack);
	EXPECT_EQ(cons_deep.size(), 2);
	cons_deep = con1.all_conflicts(John);
	EXPECT_TRUE(cons_deep.empty());
	cons_deep = con2.all_conflicts(John);
	EXPECT_EQ(cons_deep.size(), 4);		// John is in conflict with all the others due to cascading
}

TEST_F(ConflictsTest, Remove)
{
	con1.remove(Joe);
	EXPECT_EQ(con1.size(), 2);
	EXPECT_FALSE(con1.in_conflict(Joe));
	con2.remove(Joe);
	EXPECT_FALSE(con2.in_conflict(Kyle, John));
}

TEST_F(ConflictsTest, Conflicts)
{
	auto cons = con1.conflicts(Kyle);
	EXPECT_EQ(cons.size(), 2);
	cons = con2.conflicts(Kyle);
	EXPECT_EQ(cons.size(), 1);		// only direct conflicts
}

TEST_F(ConflictsTest, Clear)
{
	con0.clear();
	con1.clear();
	EXPECT_EQ(con0.size(), 0);
	EXPECT_EQ(con1.size(), 0);
	EXPECT_TRUE(con1.empty());
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
