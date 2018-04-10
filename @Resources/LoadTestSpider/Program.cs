using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Threading;

namespace LoadTestSpider
{
    class Program
    {
        static void ThreadProc()
        {
            Crawler crawler = new Crawler("http://192.168.1.10:8081/");
            crawler.Start();
        }

        static void Main(string[] args)
        {
            for (int thread = 0; thread < 10; thread++)
            {
                Thread worker = new Thread(ThreadProc);
                worker.Start();
                System.Threading.Thread.Sleep(1000);
            }

            Console.ReadLine();
        }
    }
}
