using System;
using System.Diagnostics;
using System.Text;
using System.Windows.Forms;
using rr = Re2.Net;
using nn = System.Text.RegularExpressions;

namespace Re2.Net.Test
{
    class Test
    {
        static void Main(string[] args)
        {
            try
            {
                var regex = "(((ab)|(cd))+)((e|f)+)(gh)?(水Ǆ)";

                Console.WriteLine("Test regex:\t (((ab)|(cd))+)((e|f)+)(gh)?(水Ǆ)");
                Console.WriteLine("Search buffer:\t abcdefefef水Ǆ");
                Console.WriteLine("");

                var net      = new nn.Regex(regex);
                var netMatch = net.Match("abcdefefef水Ǆ");
                        
                var re2      = new rr.Regex(regex);
                var re2Match = re2.Match(Encoding.UTF8.GetBytes("abcdefefef水Ǆ"));

                Debug.Assert(netMatch.Index == re2Match.Index);
                Debug.Assert(netMatch.Value == re2Match.Value);
                Debug.Assert(netMatch.Groups.Count == re2Match.Groups.Count);
                Debug.Assert(((nn.Group)netMatch == netMatch.Groups[0]) == ((rr.Group)re2Match == re2Match.Groups[0]));

                for(int i = 0; i < netMatch.Groups.Count; i++)
                {
                    Console.WriteLine(String.Format("Group {0}: .NET name {1}, RE2 name {2}",
                                            /* 0 */ i,
                                            /* 1 */ netMatch.Groups[i].Value == "" ? "<no name>" : netMatch.Groups[i].Value,
                                            /* 2 */ re2Match.Groups[i].Value == "" ? "<no name>" : re2Match.Groups[i].Value));

                    var last = netMatch.Groups[i].Captures.Count - 1;
                    if(last < 0)
                    {
                        Console.WriteLine("");
                        continue;
                    }

                    Console.WriteLine("    RE2 returned 1 capture: \n        Capture 1: " + re2Match.Groups[i].Captures[0].Value);
                    var sb = new StringBuilder(String.Format("    .NET returned {0} {1}: ",
                                                     /* 0 */ last + 1,
                                                     /* 1 */ last == 0 ? "capture" : "captures"));

                    int cap = 0;
                    foreach(nn.Capture c in netMatch.Groups[i].Captures)
                        sb.Append("\n        Capture " + ++cap + ": " + c.Value);

                    Console.WriteLine(sb.ToString());
                    Console.WriteLine("");
                }
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message + ex.StackTrace);
            }

            Console.ReadLine();
        }
    }
}
