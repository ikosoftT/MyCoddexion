import Link from "next/link";
import { contact } from "@/lib/siteData";

export default function Footer() {
  return (
    <footer className="mt-auto border-t border-zinc-800 bg-zinc-950">
      <div className="mx-auto grid max-w-7xl gap-6 px-4 py-10 text-sm text-zinc-300 sm:grid-cols-3">
        <div>
          <h3 className="mb-2 text-base font-semibold text-white">KroozTV US</h3>
          <p>Premium IPTV streaming with live channels, VOD, and 24/7 support.</p>
        </div>
        <div>
          <h3 className="mb-2 text-base font-semibold text-white">Quick Links</h3>
          <div className="flex flex-col gap-1">
            <Link href="/pricing">Pricing</Link>
            <Link href="/setup-installation">Setup</Link>
            <Link href="/blog">Blog</Link>
            <Link href="/faq">FAQ</Link>
            <Link href="/privacy">Privacy Policy</Link>
            <Link href="/terms">Terms of Service</Link>
          </div>
        </div>
        <div>
          <h3 className="mb-2 text-base font-semibold text-white">Support</h3>
          <p>WhatsApp: {contact.whatsapp}</p>
          <p>Email: {contact.email}</p>
          <p>{contact.hours}</p>
        </div>
      </div>
      <div className="border-t border-zinc-800 px-4 py-4 text-center text-xs text-zinc-500">
        <p className="mb-1">
          Please stream responsibly and comply with your local laws and content rights.
        </p>
        © {new Date().getFullYear()} KroozTV US. All rights reserved.
      </div>
    </footer>
  );
}
