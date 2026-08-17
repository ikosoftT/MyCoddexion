import Image from "next/image";
import Link from "next/link";
import BlogCard from "@/components/BlogCard";
import FeatureGrid from "@/components/FeatureGrid";
import { blogPosts, features, testimonials } from "@/lib/siteData";

export default function HomePage() {
  return (
    <div className="space-y-16 py-10">
      <section className="container rounded-2xl border border-zinc-800 bg-gradient-to-r from-zinc-950 via-zinc-900 to-red-950 p-8 lg:p-14">
        <h1 className="text-3xl font-bold text-white md:text-5xl">Premium Streaming Redefined - 16,000+ Live TV Channels</h1>
        <p className="mt-4 max-w-2xl text-zinc-300">Watch Anywhere, Anytime - 4K/HD Quality</p>
        <div className="mt-6 flex flex-wrap gap-3">
          <Link href="/pricing" className="min-h-11 rounded bg-red-600 px-5 py-3 font-semibold text-white">Start Free 24hr Trial</Link>
          <Link href="/channels-vod" className="min-h-11 rounded border border-zinc-600 px-5 py-3 text-white">Explore Library</Link>
        </div>
      </section>

      <section className="container space-y-6">
        <h2 className="text-2xl font-semibold">Key Features</h2>
        <FeatureGrid items={features} />
      </section>

      <section className="container grid items-center gap-6 lg:grid-cols-2">
        <Image src="https://images.pexels.com/photos/1201996/pexels-photo-1201996.jpeg" alt="Streaming devices" width={1000} height={700} className="rounded-xl object-cover" />
        <div>
          <h2 className="text-2xl font-semibold">Why Choose KroozTV</h2>
          <ul className="mt-4 list-disc space-y-2 pl-5 text-zinc-300">
            <li>Fast setup with M3U and Xtream credentials</li>
            <li>Stable streams for sports, news, and entertainment</li>
            <li>Multi-device compatibility with 24/7 support</li>
          </ul>
        </div>
      </section>

      <section className="container space-y-6">
        <div className="flex items-center justify-between">
          <h2 className="text-2xl font-semibold">Latest News</h2>
          <Link href="/blog" className="text-sm text-red-400">View all posts</Link>
        </div>
        <div className="grid gap-4 md:grid-cols-2 xl:grid-cols-4">
          {blogPosts.slice(0, 4).map((post) => <BlogCard key={post.slug} post={post} />)}
        </div>
      </section>

      <section className="container space-y-4">
        <h2 className="text-2xl font-semibold">Testimonials</h2>
        <div className="grid gap-3 md:grid-cols-2 xl:grid-cols-3">
          {testimonials.map((item) => (
            <article key={item} className="rounded-xl border border-zinc-800 bg-zinc-900 p-4">
              <p className="mb-2 text-yellow-400">★★★★★</p>
              <p className="text-zinc-200">{item}</p>
            </article>
          ))}
        </div>
      </section>

      <section className="container rounded-2xl border border-zinc-800 bg-zinc-900 p-8 text-center">
        <h2 className="text-2xl font-semibold">Join 50,000+ Happy Streamers Today</h2>
        <div className="mt-5 flex flex-wrap justify-center gap-3">
          <Link href="/pricing" className="min-h-11 rounded bg-red-600 px-5 py-3 font-semibold text-white">Get Started</Link>
          <Link href="/setup-installation" className="min-h-11 rounded border border-zinc-600 px-5 py-3">Watch Demo</Link>
        </div>
      </section>
    </div>
  );
}
