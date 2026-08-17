export default function robots() {
  return {
    rules: [
      {
        userAgent: "*",
        allow: "/",
        disallow: ["/admin", "/api/private"],
      },
    ],
    sitemap: "https://krooztvus.us/sitemap.xml",
  };
}
