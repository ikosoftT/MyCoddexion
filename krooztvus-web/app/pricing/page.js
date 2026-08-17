import PricingCard from "@/components/PricingCard";
import Accordion from "@/components/Accordion";
import { pricingPlans } from "@/lib/siteData";

export const metadata = {
  title: "Pricing",
  description: "Simple, transparent IPTV pricing plans for KroozTV US.",
};

const faqItems = [
  { q: "Is free trial available?", a: "Yes, a 24-hour trial is available on request." },
  { q: "Which payment methods are accepted?", a: "Major cards and supported digital payments." },
  { q: "Can I cancel?", a: "Yes, cancellation can be requested anytime." },
  { q: "Do you provide refunds?", a: "Refund requests are reviewed per Terms of Service." },
];

export default function PricingPage() {
  return (
    <div className="container space-y-10 py-10">
      <section>
        <h1 className="text-3xl font-bold">Simple, Transparent Pricing</h1>
        <p className="mt-2 text-zinc-300">Choose the plan that fits your needs.</p>
      </section>

      <section className="grid gap-4 sm:grid-cols-2 xl:grid-cols-4">
        {pricingPlans.map((plan) => (
          <PricingCard key={plan.months} plan={plan} />
        ))}
      </section>

      <section className="overflow-auto rounded-xl border border-zinc-800">
        <table className="min-w-full text-sm">
          <thead className="bg-zinc-900 text-white">
            <tr>
              <th className="px-3 py-2 text-left">Feature</th>
              {pricingPlans.map((plan) => <th key={plan.months} className="px-3 py-2">{plan.months}M</th>)}
            </tr>
          </thead>
          <tbody className="text-zinc-300">
            {["Devices", "Quality", "Channels", "VOD", "Sports", "Support"].map((feature) => (
              <tr key={feature} className="border-t border-zinc-800">
                <td className="px-3 py-2 font-medium">{feature}</td>
                {pricingPlans.map((plan) => <td key={`${feature}-${plan.months}`} className="px-3 py-2 text-center">✓</td>)}
              </tr>
            ))}
          </tbody>
        </table>
      </section>

      <section>
        <h2 className="mb-4 text-2xl font-semibold">Pricing FAQ</h2>
        <Accordion items={faqItems} />
      </section>
    </div>
  );
}
