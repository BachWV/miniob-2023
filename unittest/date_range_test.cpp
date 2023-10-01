#include "gtest/gtest.h"
#include "sql/parser/parser_helper_func.h"

TEST(DateRangeTest, Test1)
{
  char *valid_dates[] = {
      "'2001-11-29'",
      "'1978-03-09'",
      "'1970-1-1'",
      "'2038-1-19'",
  };

  char *invalid_dates[] = {
      "'2017-2-29'",
      "'1800-21-29'",
      "'2017-12-32'",
      "'3058-11-31'",
  };

  for (int i = 0; i < 4; i++) {
    int time;
    EXPECT_TRUE(CheckTimeRange(valid_dates[i], time))<<" date = " << valid_dates[i]<<endl;
  }
  for (int i = 0; i < 4; i++) {
    int time;
    EXPECT_FALSE(CheckTimeRange(invalid_dates[i], time))<<" date = " << invalid_dates[i]<<endl;
  }

}