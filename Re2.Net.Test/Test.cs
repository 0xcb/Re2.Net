using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using nn = System.Text.RegularExpressions;
using rr = Re2.Net;

namespace Re2.Net.Test
{
    class Test
    {
        private class TestCase
        {
            public  string Pattern;
            public  int    Re2ByteMatchCount   = 0;
            public  int    Re2StringMatchCount = 0;
            public  int    NETMatchCount       = 0;

            private List<double> netResults       = new List<double>();
            private List<double> re2ByteResults   = new List<double>();
            private List<double> re2StringResults = new List<double>();

            public TestCase(string pattern)
            {
                Pattern = pattern;
            }

            public void AddNETResult(double time)
            {
                netResults.Add(time);
            }

            public void AddRe2ByteResult(double time)
            {
                re2ByteResults.Add(time);
            }

            public void AddRe2StringResult(double time)
            {
                re2StringResults.Add(time);
            }

            private double getResultMedian(List<double> results)
            {
                if(results.Count == 0)
                    return double.NaN;

                results.Sort();

                if((results.Count & 1) == 1)
                    return results[results.Count/2];
                else
                    return (results[results.Count/2] + results[results.Count/2 - 1]) / 2d;
            }

            public double GetNETResultMedian()
            {
                return getResultMedian(netResults);
            }

            public double GetRe2ByteResultMedian()
            {
                return getResultMedian(re2ByteResults);
            }

            public double GetRe2StringResultMedian()
            {
                return getResultMedian(re2StringResults);
            }

            public void Reset()
            {
                Re2ByteMatchCount   = 0;
                Re2StringMatchCount = 0;
                NETMatchCount       = 0;
                re2ByteResults      = new List<double>();
                re2StringResults    = new List<double>();
                netResults          = new List<double>();
            }
        }

        static void PrintByteVsStringResults(TestCase[] testcases)
        {
            var table = new StringBuilder("Regular Expression|Re2.Net|.NET Regex|Winner\n---|---:|---:|:---:");
            foreach(var testcase in testcases)
            {
                var re2Median = testcase.GetRe2ByteResultMedian();
                var netMedian = testcase.GetNETResultMedian();
                table.Append(
                    String.Format("\n<code>{0}</code>|{1} ms|{2} ms|{3} by **{4}x**",
                                   testcase.Pattern.Replace("|", "&#124;").Replace("](", @"]\("),
                                   re2Median.ToString(GetDoubleFormatString(re2Median)),
                                   netMedian.ToString(GetDoubleFormatString(netMedian)),
                                   re2Median > netMedian ? ".NET Regex" : "Re2.Net",
                                   (re2Median > netMedian ? re2Median/netMedian : netMedian/re2Median).ToString("0.0")
                    )
                );
            }
            Console.WriteLine(table.ToString());
        }

        static void PrintStringVsStringResults(TestCase[] testcases)
        {
            var table = new StringBuilder("Regular Expression|Re2.Net|.NET Regex|Winner\n---|---:|---:|:---:");
            foreach(var testcase in testcases)
            {
                var re2Median = testcase.GetRe2StringResultMedian();
                var netMedian = testcase.GetNETResultMedian();
                table.Append(
                    String.Format("\n<code>{0}</code>|{1} ms|{2} ms|{3} by **{4}x**",
                                   testcase.Pattern.Replace("|", "&#124;").Replace("](", @"]\("),
                                   re2Median.ToString(GetDoubleFormatString(re2Median)),
                                   netMedian.ToString(GetDoubleFormatString(netMedian)),
                                   re2Median > netMedian ? ".NET Regex" : "Re2.Net",
                                   (re2Median > netMedian ? re2Median/netMedian : netMedian/re2Median).ToString("0.0")
                    )
                );
            }
            Console.WriteLine(table.ToString());
        }

        static double TimerTicksToMilliseconds(long ticks)
        {
            return (double)ticks / (double)Stopwatch.Frequency * 1000d;
        }

        static string GetDoubleFormatString(double d)
        {
            return d >= 5     ? "0"     :
                   d >= 0.1   ? "0.0"   :
                   d >= 0.01  ? "0.00"  :
                   d >= 0.001 ? "0.000" :
                   "0.0000";
        }

        static void Main(string[] args)
        {
            try
            {
                var iterations = 5;

                var builder  = new StringBuilder();
                var haybytes = System.IO.File.ReadAllBytes(@"..\..\mtent12.txt");
                var watch    = new Stopwatch();

                watch.Start();
                var haystring  = Encoding.ASCII.GetString(haybytes);
                var encodetime = watch.Elapsed;
                watch.Reset();

                Console.WriteLine("Text length: " + haystring.Length);
                Console.WriteLine("Encoding time: " + encodetime);
                Console.WriteLine();

                var testcases = new TestCase[16] {
                    new TestCase("Twain"),
                    new TestCase("^Twain"),
                    new TestCase("Twain$"),
                    new TestCase("Huck[a-zA-Z]+|Finn[a-zA-Z]+"),
                    new TestCase("a[^x]{20}b"),
                    new TestCase("Tom|Sawyer|Huckleberry|Finn"),
                    new TestCase(".{0,3}(Tom|Sawyer|Huckleberry|Finn)"),
                    new TestCase("[a-zA-Z]+ing"),
                    new TestCase("^[a-zA-Z]{0,4}ing[^a-zA-Z]"),
                    new TestCase("[a-zA-Z]+ing$"),
                    new TestCase("^[a-zA-Z ]{5,}$"),
                    new TestCase("^.{16,20}$"),
                    new TestCase("([a-f](.[d-m].){0,2}[h-n]){2}"),
                    new TestCase("([A-Za-z]awyer|[A-Za-z]inn)[^a-zA-Z]"),
                    new TestCase(@"""[^""]{0,30}[?!\.]"""),
                    new TestCase("Tom.{10,25}river|river.{10,25}Tom")
                };

                Console.Write("Running 'First Match' test..");

                for(int i = 0; i < iterations; i++)
                    foreach(var testcase in testcases)
                    {
                        Console.Write(".");

                        var re2 = new rr.Regex(testcase.Pattern, rr.RegexOptions.Multiline);
                        var net = new nn.Regex(testcase.Pattern, nn.RegexOptions.Multiline);

                        watch.Start();
                        var re2ByteMatch = re2.Match(haybytes);
                        testcase.AddRe2ByteResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        watch.Start();
                        var re2StringMatch = re2.Match(haystring);
                        testcase.AddRe2StringResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        watch.Start();
                        var netMatch = net.Match(haystring);
                        testcase.AddNETResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        Debug.Assert(re2ByteMatch.Value == re2StringMatch.Value);
                        Debug.Assert(re2ByteMatch.Value == netMatch.Value);
                        Debug.Assert(re2StringMatch.Index == netMatch.Index);
                        Debug.Assert(re2StringMatch.Length == netMatch.Length);
                    }

                Console.WriteLine("\n\nResults:\n\n");

                PrintByteVsStringResults(testcases);
                Console.WriteLine("\n");
                PrintStringVsStringResults(testcases);

                foreach(var testcase in testcases)
                    testcase.Reset();

                Console.Write("\n\nRunning 'All Matches' test..");

                for(int i = 0; i < iterations; i++)
                    foreach(var testcase in testcases)
                    {
                        Console.Write(".");

                        var re2 = new rr.Regex(testcase.Pattern, rr.RegexOptions.Multiline);
                        var net = new nn.Regex(testcase.Pattern, nn.RegexOptions.Multiline);

                        watch.Start();
                        var re2ByteMatches = re2.Matches(haybytes);
                        // Matches() methods are lazily evaluated.
                        testcase.Re2ByteMatchCount = re2ByteMatches.Count;
                        testcase.AddRe2ByteResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        watch.Start();
                        var re2StringMatches = re2.Matches(haystring);
                        // Matches() methods are lazily evaluated.
                        testcase.Re2StringMatchCount = re2StringMatches.Count;
                        testcase.AddRe2StringResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        watch.Start();
                        var netMatches = net.Matches(haystring);
                        // Matches() methods are lazily evaluated.
                        testcase.NETMatchCount = netMatches.Count;
                        testcase.AddNETResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        Debug.Assert(re2ByteMatches.Count == re2StringMatches.Count);
                        Debug.Assert(re2ByteMatches.Count == netMatches.Count);
                    }

                Console.WriteLine("\n\nResults:\n\n");

                PrintByteVsStringResults(testcases);
                Console.WriteLine("\n");
                PrintStringVsStringResults(testcases);
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message + ex.StackTrace);
            }

            Console.ReadLine();
        }
    }
}
