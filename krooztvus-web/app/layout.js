import "./globals.css";
import Header from "@/components/Header";
import Footer from "@/components/Footer";

export const metadata = {
  metadataBase: new URL("https://krooztvus.us"),
  title: {
    default: "KroozTV US | Premium IPTV Streaming",
    template: "%s | KroozTV US",
  },
  description: "Premium IPTV streaming with 16,000+ live channels, 40,000+ VOD titles, and 24/7 support.",
  alternates: { canonical: "/" },
  openGraph: {
    title: "KroozTV US",
    description: "Premium IPTV streaming service for live TV and VOD.",
    url: "https://krooztvus.us",
    siteName: "KroozTV US",
    type: "website",
  },
  twitter: {
    card: "summary_large_image",
    title: "KroozTV US",
    description: "Live TV channels, VOD library, and easy setup across devices.",
  },
};

export default function RootLayout({ children }) {
  const organizationSchema = {
    "@context": "https://schema.org",
    "@type": "Organization",
    name: "KroozTV US",
    url: "https://krooztvus.us",
    email: "support@krooztvus.us",
    contactPoint: [{ "@type": "ContactPoint", contactType: "customer support", availableLanguage: ["English", "Spanish"] }],
  };

  return (
    <html lang="en">
      <body className="min-h-screen bg-zinc-950 text-zinc-100 antialiased">
        <script type="application/ld+json" dangerouslySetInnerHTML={{ __html: JSON.stringify(organizationSchema) }} />
        <Header />
        <main>{children}</main>
        <Footer />
      </body>
    </html>
  );
}
