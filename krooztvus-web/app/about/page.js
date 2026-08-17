import Image from "next/image";
import { contact } from "@/lib/siteData";

export const metadata = {
  title: "About",
  description: "Learn about KroozTV US mission, values, and support contacts.",
};

export default function AboutPage() {
  return (
    <div className="container space-y-10 py-10">
      <section className="grid gap-6 lg:grid-cols-2">
        <Image src="https://images.pexels.com/photos/3183150/pexels-photo-3183150.jpeg" alt="Team" width={1000} height={700} className="rounded-xl object-cover" />
        <div>
          <h1 className="text-3xl font-bold">Who We Are</h1>
          <p className="mt-3 text-zinc-300">KroozTV US delivers a premium IPTV experience focused on reliability, support, and user-friendly setup.</p>
          <p className="mt-3 text-zinc-300">Our team helps customers stream confidently across devices with clear onboarding and responsive support.</p>
        </div>
      </section>

      <section>
        <h2 className="mb-4 text-2xl font-semibold">Mission & Values</h2>
        <div className="grid gap-4 md:grid-cols-3">
          {[
            ["Quality", "Stable streams and consistent uptime."],
            ["Reliability", "Clear setup flows and accountable support."],
            ["Innovation", "Modern player compatibility and performance tuning."],
          ].map(([title, text]) => (
            <article key={title} className="rounded-xl border border-zinc-800 bg-zinc-900 p-5">
              <h3 className="font-semibold">{title}</h3>
              <p className="mt-2 text-sm text-zinc-300">{text}</p>
            </article>
          ))}
        </div>
      </section>

      <section className="rounded-xl border border-zinc-800 bg-zinc-900 p-6">
        <h2 className="text-2xl font-semibold">Contact Information</h2>
        <p className="mt-3">WhatsApp: {contact.whatsapp}</p>
        <p>Email: {contact.email}</p>
        <p>Hours: {contact.hours}</p>
      </section>
    </div>
  );
}
