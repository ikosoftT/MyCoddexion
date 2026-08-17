export default function PricingCard({ plan }) {
  return (
    <article className={`rounded-xl border p-5 ${plan.months === 12 ? "border-yellow-500 bg-zinc-900" : "border-zinc-800 bg-zinc-950"}`}>
      {plan.badge && <p className="mb-3 inline-block rounded bg-yellow-500 px-2 py-1 text-xs font-semibold text-black">{plan.badge}</p>}
      <h3 className="text-xl font-semibold text-white">{plan.months} Month{plan.months > 1 ? "s" : ""}</h3>
      <p className="mt-2 text-3xl font-bold text-red-500">${plan.price}.00</p>
      <ul className="mt-4 space-y-1 text-sm text-zinc-300">
        <li>Up to 5 Devices</li>
        <li>4K/HD/FHD/SD</li>
        <li>16,000+ Channels + VOD</li>
        <li>24/7 Priority Support</li>
      </ul>
      <button className="mt-5 min-h-11 w-full rounded bg-red-600 px-4 py-2 font-semibold text-white hover:bg-red-500">
        Subscribe Now
      </button>
    </article>
  );
}
