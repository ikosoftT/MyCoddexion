export default function FeatureGrid({ items }) {
  return (
    <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
      {items.map((feature) => (
        <div key={feature} className="rounded-xl border border-zinc-800 bg-zinc-900 p-5">
          <p className="font-medium text-white">{feature}</p>
        </div>
      ))}
    </div>
  );
}
