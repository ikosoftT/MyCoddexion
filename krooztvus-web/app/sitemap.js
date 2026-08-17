const routes = ["", "/pricing", "/channels-vod", "/setup-installation", "/about", "/blog", "/faq", "/contact", "/privacy", "/terms"];

export default function sitemap() {
  return routes.map((route) => ({
    url: `https://krooztvus.us${route}`,
    lastModified: new Date(),
  }));
}
