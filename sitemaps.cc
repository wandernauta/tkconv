#include "sitemaps.hh"
#include "support.hh"
#include "pugixml.hpp"
#include "inja.hpp"
#include <string>
using namespace std;

void createSiteMapHandlers(SimpleWebSystem& sws, ThingPool<SQLiteWriter>& tp)
{  
  sws.d_svr.Get("/sitemap-(20\\d\\d).txt", [&tp](const auto& req, auto& res) {
    auto sqlw=tp.getLease();
    string year = req.matches[1];
    year += "-%";
    auto nums=sqlw->queryT("select nummer from Document where datum like ?", {year});
    string resp;
    for(auto& n : nums) {
      resp += fmt::format("https://berthub.eu/tkconv/document.html?nummer={}\n", get<string>(n["nummer"]));
    }
    nums=sqlw->queryT("select vergadering.id from vergadering,verslag where vergaderingid=vergadering.id and status != 'Casco' and datum like ? group by vergadering.id", {year});
    for(auto& n : nums) {
      resp += fmt::format("https://berthub.eu/tkconv/verslag.html?vergaderingid={}\n", get<string>(n["id"]));
    }
    
    res.set_content(resp, "text/plain");
  });

  sws.d_svr.Get("/sitemap-(20\\d\\d)-H([12]).txt", [&tp](const auto& req, auto& res) {
    auto sqlw=tp.getLease();
    string year = req.matches[1];
    string h = req.matches[2];

    string fromDate, toDate;
    if(h=="1") {
      fromDate = year+"-01-01";
      toDate = year+"-07-01";
    }
    if(h=="2") {
      fromDate = year+"-07-02";
      toDate = year+"-12-31";
    }
    cout<<"fromDate: "<<fromDate<<", toDate: "<<toDate<<endl;
    
    auto nums=sqlw->queryT("select nummer from Document where datum >= ? and datum <= ?", {fromDate, toDate});
    string resp;
    for(auto& n : nums) {
      resp += fmt::format("https://berthub.eu/tkconv/document.html?nummer={}\n", get<string>(n["nummer"]));
    }
    nums=sqlw->queryT("select vergadering.id from vergadering,verslag where vergaderingid=vergadering.id and status != 'Casco' and datum >= ? and datum <= ? group by vergadering.id", {fromDate, toDate});
    for(auto& n : nums) {
      resp += fmt::format("https://berthub.eu/tkconv/verslag.html?vergaderingid={}\n", get<string>(n["id"]));
    }
    
    res.set_content(resp, "text/plain");
  });

  sws.d_svr.Get("/sitemap-(20\\d\\d-\\d\\d).txt", [&tp](const auto& req, auto& res) {
    auto sqlw = tp.getLease();
    string month = req.matches[1];
    month += "-%";
    auto nums=sqlw->queryT("select nummer from Document where datum like ?", {month});
    string resp;
    for(auto& n : nums) {
      resp += fmt::format("https://berthub.eu/tkconv/document.html?nummer={}\n", get<string>(n["nummer"]));
    }
    nums = sqlw->queryT("select vergadering.id from vergadering,verslag where vergaderingid=vergadering.id and status != 'Casco' and datum like ? group by vergadering.id", {month});
    for(auto& n : nums) {
      resp += fmt::format("https://berthub.eu/tkconv/verslag.html?vergaderingid={}\n", get<string>(n["id"]));
    }

    nums=sqlw->queryT("select id from OODocument where openbaarmakingsdatum like ?", {month});
    for(auto& n : nums) {
      // https://berthub.eu/tkconv/oo.html?nummer=oep-ob-57e776c487884bf44a859823c3f37231c183b9c6
      resp += fmt::format("https://berthub.eu/tkconv/oo.html?nummer={}\n", get<string>(n["id"]));
    }
    
    res.set_content(resp, "text/plain");
  });

  /*
<?xml version="1.0" encoding="UTF-8"?>
<sitemapindex xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">
  <sitemap>
    <loc>https://www.example.com/sitemap1.xml.gz</loc>
    <lastmod>2024-08-15</lastmod>
  </sitemap>
  <sitemap>
    <loc>https://www.example.com/sitemap2.xml.gz</loc>
    <lastmod>2022-06-05</lastmod>
  </sitemap>
</sitemapindex>
  */
  
  sws.d_svr.Get("/sitemap.xml", [&tp](const auto& req, auto& res) {
    string content=R"(<?xml version="1.0" encoding="UTF-8"?><sitemapindex xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">)";

    auto sqlw = tp.getLease();
    auto months = sqlw->queryT("select substr(datum,0,8) m, max(substr(datum,0,11)) mmax from Document group by 1"); // 2026-06..
    for(const auto& m : months) {
      content += fmt::format("<sitemap><loc>https://berthub.eu/tkconv/sitemap-{}.txt</loc><lastmod>{}</lastmod></sitemap>\n",
			     eget(m, "m"),
			     eget(m, "mmax")
			     );
    }
    content += "</sitemapindex>\n";
    res.set_content(content, "application/xml");    
  });
}
