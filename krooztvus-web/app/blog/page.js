import BlogCard from "@/components/BlogCard";
import { blogPosts } from "@/lib/siteData";

export const metadata = {
  title: "Blog",
  description: "IPTV setup tips, reviews, and streaming best practices from KroozTV US.",
};

export default function BlogPage() {
  return (
    <div className="container space-y-8 py-10">
      <section>
        <h1 className="text-3xl font-bold">KroozTV Blog</h1>
        <p className="mt-2 text-zinc-300">Setup tips, reviews, industry updates, and practical streaming guides.</p>
      </section>

      <section className="grid gap-3 sm:grid-cols-[2fr_1fr]">
        <input placeholder="Search posts" className="rounded border border-zinc-700 bg-zinc-900 px-3 py-2" />
        <div className="rounded border border-zinc-700 bg-zinc-900 px-3 py-2 text-sm text-zinc-300">
          Categories: Setup Tips, Reviews, Industry News, How-To
        </div>
      </section>

      <section className="grid gap-4 md:grid-cols-2 xl:grid-cols-3">
        {blogPosts.map((post) => <BlogCard key={post.slug} post={post} />)}
      </section>

      <section className="flex justify-center gap-2 text-sm">
        <span className="rounded border border-zinc-700 px-3 py-2">1</span>
        <span className="rounded border border-zinc-700 px-3 py-2">2</span>
        <span className="rounded border border-zinc-700 px-3 py-2">3</span>
      </section>
    </div>
  );
}
