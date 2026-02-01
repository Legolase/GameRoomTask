#include <RevenuerManager.hpp>

#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

#include <log.hpp>

namespace {
std::string run(const std::string& input)
{
  std::stringstream in;
  std::stringstream out;

  in << input;

  task::RevenuerManager manager(in, out);

  try {
    manager.process();
  } catch (const std::runtime_error& e) {
    out << e.what();
  }

  return out.str();
}
} // namespace

TEST(Base, Example)
{
  std::string input = R"x(3
09:00 19:00
10
08:48 1 client1
09:41 1 client1
09:48 1 client2
09:52 3 client1
09:54 2 client1 1
10:25 2 client2 2
10:58 1 client3
10:59 2 client3 3
11:30 1 client4
11:35 2 client4 2
11:45 3 client4
12:33 4 client1
12:43 4 client2
15:52 4 client4
)x";

  std::string output = R"x(09:00
08:48 1 client1
08:48 13 NotOpenYet
09:41 1 client1
09:48 1 client2
09:52 3 client1
09:52 13 ICanWaitNoLonger!
09:54 2 client1 1
10:25 2 client2 2
10:58 1 client3
10:59 2 client3 3
11:30 1 client4
11:35 2 client4 2
11:35 13 PlaceIsBusy
11:45 3 client4
12:33 4 client1
12:33 12 client4 1
12:43 4 client2
15:52 4 client4
19:00 11 client3
19:00
1 70 05:58
2 30 02:18
3 90 08:01
)x";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, EmptyFile)
{
  std::string input = "";

  std::string output = "";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, BadTime1)
{
  std::string input = R"x(3
00:00 00:0O
15)x";

  std::string output = "00:00 00:0O";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, BadTime2)
{
  std::string input = R"x(3
00:00
15)x";

  std::string output = "00:00";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, BadTime3)
{
  std::string input = R"x(3
09:60 13:80
15)x";

  std::string output = "09:60 13:80";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, ZeroCountOfTable)
{
  std::string input = R"x(0
00:00 00:00
15)x";

  std::string output = "0";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, NonPositiveCostPerHour)
{
  std::string input = R"x(1
00:00 00:00
0)x";

  std::string output = "0";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, InvalidClientName)
{
  std::string input = R"x(1
09:00 21:00
10
10:00 1 k%eer)x";

  std::string output = "10:00 1 k%eer";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, EmptyClientName)
{
  std::string input = R"x(1
09:00 21:00
10
10:00 2   )x";

  std::string output = "10:00 2   ";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, EmptyTableID)
{
  std::string input = R"x(1
09:00 21:00
10
10:00 2 name   )x";

  std::string output = "10:00 2 name   ";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, OneDelimiterInHeader)
{
  std::string input = R"x(1
09:00  21:00
10
10:00 2  name 5)x";

  std::string output = "09:00  21:00";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, OneDelimiterInEvent1)
{
  std::string input = R"x(1
09:00 21:00
10
 10:00 2 name 5)x";

  std::string output = " 10:00 2 name 5";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, OneDelimiterInEvent2)
{
  std::string input = R"x(1
09:00 21:00
10
10:00  2 name 5)x";

  std::string output = "10:00  2 name 5";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, OneDelimiterInEvent3)
{
  std::string input = R"x(1
09:00 21:00
10
10:00 2  name 5)x";

  std::string output = "10:00 2  name 5";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, OneDelimiterInEvent4)
{
  std::string input = R"x(1
09:00 21:00
10
10:00 2 name  5)x";

  std::string output = "10:00 2 name  5";

  EXPECT_EQ(output, run(input));
}

TEST(Syntax, OneDelimiterInEvent5)
{
  std::string input = R"x(1
09:00 21:00
10
10:00 2 name 5 )x";

  std::string output = "10:00 2 name 5 ";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, TableIDOutOfRange)
{
  std::string input = R"x(1
09:00 21:00
10
09:00 1 client1
10:00 2 client1 5)x";

  std::string output = "10:00 2 client1 5";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, TableIDInRange)
{
  std::string input = R"x(1
09:00 21:00
10
09:00 1 client1
10:00 2 client1 1)x";

  std::string output = R"x(09:00
09:00 1 client1
10:00 2 client1 1
21:00 11 client1
21:00
1 110 11:00
)x";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, UnorderedEvent)
{
  std::string input = R"x(1
09:00 21:00
10
10:00 1 client1
09:59 2 client1 1)x";

  std::string output = "09:59 2 client1 1";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, InvalidEventFormatArrive)
{
  std::string input = R"x(1
09:00 21:00
10
09:00 1 client1 3
10:00 2 client1 1)x";

  std::string output = "09:00 1 client1 3";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, InvalidEventFormatSitTable)
{
  std::string input = R"x(1
09:00 21:00
10
09:00 1 client1
10:00 2 client)x";

  std::string output = "10:00 2 client";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, InvalidEventFormatWaiting)
{
  std::string input = R"x(1
09:00 21:00
10
09:00 1 client1
10:00 2 client1 1
10:05 1 client2
10:05 3)x";

  std::string output = "10:05 3";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, InvalidEventFormatLeave)
{
  std::string input = R"x(1
09:00 21:00
10
09:00 1 client1
10:00 2 client1 1
10:05 1 client2
10:10 4 
)x";

  std::string output = "10:10 4 ";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, TransferTables)
{
  std::string input = R"x(3
09:00 21:00
15
09:25 1 nikita
09:30 2 nikita 1
09:40 1 vasya
09:45 2 vasya 2
10:45 2 nikita 3
11:00 2 vasya 1
11:01 4 vasya
13:02 4 nikita
)x";

  std::string output = R"x(09:00
09:25 1 nikita
09:30 2 nikita 1
09:40 1 vasya
09:45 2 vasya 2
10:45 2 nikita 3
11:00 2 vasya 1
11:01 4 vasya
13:02 4 nikita
21:00
1 45 01:16
2 30 01:15
3 45 02:17
)x";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, NotOpenComputerClub)
{
  std::string input = R"x(3
09:00 21:00
15
08:01 1 nikita
08:01 3 nikita
08:02 2 nikita 1
08:02 4 nikita
)x";

  std::string output = R"x(09:00
08:01 1 nikita
08:01 13 NotOpenYet
08:01 3 nikita
08:01 13 ClientUnknown
08:02 2 nikita 1
08:02 13 ClientUnknown
08:02 4 nikita
08:02 13 ClientUnknown
21:00
1 0 00:00
2 0 00:00
3 0 00:00
)x";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, BusyPlace)
{
  std::string input = R"x(2
09:00 21:00
40
09:00 1 nikita
09:05 2 nikita 1
09:20 1 vova
09:20 2 vova 2
09:30 2 nikita 1
09:30 2 nikita 2
10:00 4 nikita
10:21 4 vova
)x";

  std::string output = R"x(09:00
09:00 1 nikita
09:05 2 nikita 1
09:20 1 vova
09:20 2 vova 2
09:30 2 nikita 1
09:30 13 PlaceIsBusy
09:30 2 nikita 2
09:30 13 PlaceIsBusy
10:00 4 nikita
10:21 4 vova
21:00
1 40 00:55
2 80 01:01
)x";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, ClientUnknown)
{
  std::string input = R"x(2
09:00 21:00
40
12:00 3 nikita
12:00 2 nikita 1
12:00 4 nikita
12:00 1 nikita
12:00 4 nikita
12:00 4 nikita
)x";

  std::string output = R"x(09:00
12:00 3 nikita
12:00 13 ClientUnknown
12:00 2 nikita 1
12:00 13 ClientUnknown
12:00 4 nikita
12:00 13 ClientUnknown
12:00 1 nikita
12:00 4 nikita
12:00 4 nikita
12:00 13 ClientUnknown
21:00
1 0 00:00
2 0 00:00
)x";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, ClientWait)
{
  std::string input = R"x(2
09:00 21:00
40
12:00 1 nikita
12:05 2 nikita 1
12:10 1 seva
12:15 2 seva 2
12:30 1 peta
12:31 2 peta 1
12:32 2 peta 2
12:40 3 peta
12:40 1 margarita
12:42 3 margarita
12:47 1 lida
12:50 3 lida
13:30 4 nikita
14:00 4 seva
)x";

  std::string output = R"x(09:00
12:00 1 nikita
12:05 2 nikita 1
12:10 1 seva
12:15 2 seva 2
12:30 1 peta
12:31 2 peta 1
12:31 13 PlaceIsBusy
12:32 2 peta 2
12:32 13 PlaceIsBusy
12:40 3 peta
12:40 1 margarita
12:42 3 margarita
12:47 1 lida
12:50 3 lida
12:50 11 lida
13:30 4 nikita
13:30 12 peta 1
14:00 4 seva
14:00 12 margarita 2
21:00 11 margarita
21:00 11 peta
21:00
1 400 08:55
2 360 08:45
)x";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, ClientLeave)
{
  std::string input = R"x(3
09:00 21:00
25
09:00 1 nikita
09:05 1 danya
09:10 1 alex
09:15 2 nikita 2
09:15 2 danya 1
09:15 2 alex 3
10:00 1 dasha
10:05 1 nika
10:10 1 olya
10:15 1 natasha
10:15 3 dasha
10:15 3 nika
10:15 3 olya
10:15 3 natasha
10:15 4 seva
12:00 4 nikita
13:00 4 danya
14:00 4 alex
15:00 4 olya
16:00 4 nika
17:00 4 dasha
)x";

  std::string output = R"x(09:00
09:00 1 nikita
09:05 1 danya
09:10 1 alex
09:15 2 nikita 2
09:15 2 danya 1
09:15 2 alex 3
10:00 1 dasha
10:05 1 nika
10:10 1 olya
10:15 1 natasha
10:15 3 dasha
10:15 3 nika
10:15 3 olya
10:15 3 natasha
10:15 11 natasha
10:15 4 seva
10:15 13 ClientUnknown
12:00 4 nikita
12:00 12 dasha 2
13:00 4 danya
13:00 12 nika 1
14:00 4 alex
14:00 12 olya 3
15:00 4 olya
16:00 4 nika
17:00 4 dasha
21:00
1 175 06:45
2 200 07:45
3 150 05:45
)x";

  EXPECT_EQ(output, run(input));
}

TEST(Logic, AlphabeticalOrder)
{
  std::string input = R"x(3
09:00 10:00
25
09:30 1 nikita
09:30 2 nikita 2
09:30 1 lesha
09:30 1 marina
09:30 1 sveta
09:30 1 alex
)x";

  std::string output = R"x(09:00
09:30 1 nikita
09:30 2 nikita 2
09:30 1 lesha
09:30 1 marina
09:30 1 sveta
09:30 1 alex
10:00 11 alex
10:00 11 lesha
10:00 11 marina
10:00 11 nikita
10:00 11 sveta
10:00
1 0 00:00
2 25 00:30
3 0 00:00
)x";

  EXPECT_EQ(output, run(input));
}

TEST(SuccessTests, Example1)
{
  std::string input = R"x(1
09:00 21:00
10
09:00 1 client1
10:00 2 client1 1
10:05 1 client2
10:10 4 cli
)x";

  std::string output = R"x(09:00
09:00 1 client1
10:00 2 client1 1
10:05 1 client2
10:10 4 cli
10:10 13 ClientUnknown
21:00 11 client1
21:00 11 client2
21:00
1 110 11:00
)x";

  EXPECT_EQ(output, run(input));
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}