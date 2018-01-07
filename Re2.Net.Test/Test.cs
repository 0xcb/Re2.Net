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
                {
                    Console.WriteLine("Running issue #1 test ...");
                    string source = "red car white car";
                    string pattern = @"(\w+)\s+(car)";
                    var regex = new rr.Regex(pattern);
                    MatchCollection matches = regex.Matches(source);
                    Debug.Assert(matches.Count == 2);
                    Debug.Assert(matches[0].Value == "red car");
                    Debug.Assert(matches[1].Value == "white car");
                    Debug.Assert(matches[0].Index == 0);
                    Debug.Assert(matches[1].Index == 8);
                    Console.WriteLine("\t... Success.\n");


                    Console.WriteLine("Running issue #2 test ...");
                    string s = "123";
                    var re2NetRegex = new rr.Regex(@"\d*");
                    Debug.Assert(re2NetRegex.Match(s, 1).Index == 1);
                    Debug.Assert(re2NetRegex.Match(s, 1).Length == 2);
                    Debug.Assert(re2NetRegex.Match(s, 1).Value == "23");
                    Console.WriteLine("\t... Success.\n");

                    Console.WriteLine("Running issue #3 test ...");
                    int success = 0;
                    for(uint i = 0; i < 100; ++i)
                    {
                        using(var re2 = new Regex("a"))
                        {
                            Match match = re2.Match("a");
                            if(match.Success) success++;
                        }
                    }
                    Debug.Assert(success == 100);
                    Console.WriteLine("\t... Success.\n");
                }

                {
                    Console.WriteLine("Running simple tests ...");
                    // Five matches, each with a value of "".
                    Debug.Assert(Regex.Matches("xxxx", "").Count == 5);
                    // Accessing an array (technically, the default Item property) with a negative index? Yup!
                    Debug.Assert(Regex.Matches("xxxx", "")[0].Groups[-1].Value == "");
                    // The returned Match containes a Group collection, and the first item in the Group collection is the Match.
                    Match match = Regex.Match("abcd", "abcd");
                    Debug.Assert((Group)match == match.Groups[0]);
                    // .NET allows invalid UTF-16 strings (including freestanding codepoints).
                    Debug.Assert(Regex.Match("\xD800", "\xD800").Length == 1);
                    // The translation isn't consistent for invalid strings, though.
                    Debug.Assert(Regex.Match(Encoding.UTF8.GetBytes("\xD800"), "\xD800") == Match.Empty);
                    // 2-byte UTF-16 to 3-byte UTF-8.
                    Debug.Assert(Regex.Match("水Ǆ", "水Ǆ").Length == 2);
                    Debug.Assert(Regex.Match(Encoding.UTF8.GetBytes("水Ǆ"), "水Ǆ").Length == 5);
                    // To BMP and beyond.
                    Debug.Assert(Regex.Match("xxx𠜎𠜱𠝹𠱓", "𠝹𠱓").Index == 5);
                    Debug.Assert(Regex.Match("xxx𠜎𠜱𠝹𠱓", "𠜎𠜱𠝹𠱓").Length == 4);
                    Console.WriteLine("\t... Success.\n");
                }

                {
                    Console.WriteLine("Running performance tests ...\n");

                    var builder = new StringBuilder();
                    var haybytes = System.IO.File.ReadAllBytes(@"..\..\mtent12.txt");
                    var watch = new Stopwatch();

                    watch.Start();
                    var haystring = Encoding.ASCII.GetString(haybytes);
                    var encodetime = watch.Elapsed;
                    watch.Reset();

                    Console.WriteLine("\tText length: " + haystring.Length);
                    Console.WriteLine("\tEncoding time: " + encodetime);
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

                    Console.Write("\tRunning 'First Match' test...");


                    foreach(var testcase in testcases)
                    {
                        var re2b = new rr.Regex(testcase.Pattern, rr.RegexOptions.Multiline | rr.RegexOptions.Latin1);
                        var re2s = new rr.Regex(testcase.Pattern, rr.RegexOptions.Multiline);
                        var nets = new nn.Regex(testcase.Pattern, nn.RegexOptions.Multiline);

                        watch.Start();
                        var re2ByteMatch = re2b.Match(haybytes);
                        testcase.AddRe2ByteResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        watch.Start();
                        var re2StringMatch = re2s.Match(haystring);
                        testcase.AddRe2StringResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        watch.Start();
                        var netMatch = nets.Match(haystring);
                        testcase.AddNETResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        if(re2ByteMatch.Value != re2StringMatch.Value)
                        {
                            Console.WriteLine();
                            Console.WriteLine("\tMatch.Value: RE2 bytes failed to match RE2 string for pattern " + re2b.Pattern);
                            Console.WriteLine("\tThis is not necessarily an error and may be due to accent characters.");
                            Console.WriteLine("\tRE2 bytes value: " + re2ByteMatch.Value);
                            Console.WriteLine("\tRE2 string value: " + re2StringMatch.Value);
                            Console.WriteLine();
                        }

                        if(re2StringMatch.Value != netMatch.Value)
                        {
                            Console.WriteLine();
                            Console.WriteLine("\tMatch.Value: RE2 string failed to match .NET string for pattern " + re2b.Pattern);
                            Console.WriteLine("\tThis is not necessarily an error and may be due to accent characters.");
                            Console.WriteLine("\tRE2 string value: " + re2StringMatch.Value);
                            Console.WriteLine("\t.NET string value: " + netMatch.Value);
                            Console.WriteLine();
                        }

                        Debug.Assert(re2StringMatch.Index == netMatch.Index);
                        Debug.Assert(re2StringMatch.Length == netMatch.Length);
                    }

                    Console.WriteLine("\n\nResults:\n\n");

                    PrintByteVsStringResults(testcases);
                    Console.WriteLine("\n");
                    PrintStringVsStringResults(testcases);

                    Console.WriteLine("\n\t... Success.\n");

                    foreach(var testcase in testcases) testcase.Reset();

                    Console.Write("\n\n\tRunning 'All Matches' test...");

                    foreach(var testcase in testcases)
                    {
                        var re2b = new rr.Regex(testcase.Pattern, rr.RegexOptions.Multiline | rr.RegexOptions.Latin1);
                        var re2s = new rr.Regex(testcase.Pattern, rr.RegexOptions.Multiline);
                        var nets = new nn.Regex(testcase.Pattern, nn.RegexOptions.Multiline);

                        watch.Start();
                        var re2ByteMatches = re2b.Matches(haybytes);
                        // Matches() methods are lazily evaluated.
                        testcase.Re2ByteMatchCount = re2ByteMatches.Count;
                        testcase.AddRe2ByteResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        watch.Start();
                        var re2StringMatches = re2s.Matches(haystring);
                        // Matches() methods are lazily evaluated.
                        testcase.Re2StringMatchCount = re2StringMatches.Count;
                        testcase.AddRe2StringResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        watch.Start();
                        var netMatches = nets.Matches(haystring);
                        // Matches() methods are lazily evaluated.
                        testcase.NETMatchCount = netMatches.Count;
                        testcase.AddNETResult(TimerTicksToMilliseconds(watch.ElapsedTicks));
                        watch.Reset();

                        Debug.Assert(re2ByteMatches.Count == re2StringMatches.Count);
                        Debug.Assert(re2ByteMatches.Count == netMatches.Count);

                        for(int j = 0; j < re2ByteMatches.Count; j++)
                        {
                            if(re2ByteMatches[j].Value != re2StringMatches[j].Value)
                            {
                                Console.WriteLine();
                                Console.WriteLine("\tMatch.Value: RE2 bytes failed to match RE2 string for pattern " + re2b.Pattern);
                                Console.WriteLine("\tThis is not necessarily an error and may be due to accent characters.");
                                Console.WriteLine("\tRE2 bytes value: " + re2ByteMatches[j].Value);
                                Console.WriteLine("\tRE2 string value: " + re2StringMatches[j].Value);
                                Console.WriteLine();
                            }

                            if(re2StringMatches[j].Value != netMatches[j].Value)
                            {
                                Console.WriteLine();
                                Console.WriteLine("\tMatch.Value: RE2 string failed to match .NET string for pattern " + re2b.Pattern);
                                Console.WriteLine("\tThis is not necessarily an error and may be due to accent characters.");
                                Console.WriteLine("\tRE2 string value: " + re2StringMatches[j].Value);
                                Console.WriteLine("\t.NET string value: " + netMatches[j].Value);
                                Console.WriteLine();
                            }
                        }
                    }

                    Console.WriteLine("\n\nResults:\n\n");

                    PrintByteVsStringResults(testcases);
                    Console.WriteLine("\n");
                    PrintStringVsStringResults(testcases);

                    Console.WriteLine("\n\t... Success.\n");
                }
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message + ex.StackTrace);
            }

            GC.Collect();
            Console.WriteLine();
            Console.WriteLine("Done.");
            Console.ReadLine();
        }
    }
}
