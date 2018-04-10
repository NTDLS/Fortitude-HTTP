using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;

namespace LoadTestSpider
{
    class Crawler
    {
        /*
        public class NoKeepAlivesWebClient : WebClient
        {
            public override WebRequest GetWebRequest(Uri address)
            {
                var request = base.GetWebRequest(address);
                if (request is HttpWebRequest)
                {
                    ((HttpWebRequest)request).KeepAlive = false;
                }

                return request;
            }
        }
        */

        private Uri baseUri;

        private HashSet<String> visited = new HashSet<string>();
        private List<string> queue = new List<string>();
        private int itemCount = 0;
        WebClient client = new WebClient();

        public Crawler(string baseUri)
        {
            this.baseUri = new Uri(baseUri);
        }

        public void Start()
        {
            Download(baseUri);

            string queueItem = string.Empty;

            while ((queueItem = GetNextQueueItem()) != null)
            {
                Download(new Uri(baseUri, queueItem));
            }
        }

        string GetNextQueueItem()
        {
            lock (queue)
            {
                if (queue.Count > 0)
                {
                    string queueItem = queue[queue.Count - 1];
                    queue.RemoveAt(queue.Count - 1);
                    return queueItem;
                }
            }
            return null;
        }


        private void Download(Uri uri)
        {
            try
            {
                string pathAndQuery = uri.PathAndQuery.ToUpper();

                if (!visited.Contains(pathAndQuery))
                {
                    visited.Add(pathAndQuery);

                    Console.WriteLine(itemCount + " " + uri.PathAndQuery);
                    string html = client.DownloadString(uri);
                    itemCount++;

                    string contentType = client.ResponseHeaders["Content-Type"].ToUpper();
                    if (contentType == "TEXT/HTML")
                    {
                        List<LinkItem> links = LinkFinder.Find(html);
                        lock (queue)
                        {
                            foreach (LinkItem item in links)
                            {
                                queue.Add(item.Href);
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
    }

}
