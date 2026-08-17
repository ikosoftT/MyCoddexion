import ContactForm from "@/components/ContactForm";
import { contact } from "@/lib/siteData";

export const metadata = {
  title: "Contact Support",
  description: "Contact KroozTV US support for billing, setup, and technical help.",
};

export default function ContactPage() {
  return (
    <div className="container grid gap-6 py-10 lg:grid-cols-2">
      <section>
        <h1 className="text-3xl font-bold">Contact & Support</h1>
        <p className="mt-3 text-zinc-300">Need help with setup, billing, or streaming quality? Contact our support team.</p>
        <div className="mt-6 rounded-xl border border-zinc-800 bg-zinc-900 p-5 text-sm text-zinc-200">
          <p>WhatsApp: {contact.whatsapp}</p>
          <p>Email: {contact.email}</p>
          <p>Hours: Open 24/7</p>
        </div>
        <div className="mt-6 rounded-xl border border-zinc-800 bg-zinc-900 p-3">
          <div className="aspect-video rounded bg-zinc-800" />
          <p className="mt-2 text-xs text-zinc-400">Map placeholder for support locations.</p>
        </div>
      </section>
      <ContactForm />
    </div>
  );
}
