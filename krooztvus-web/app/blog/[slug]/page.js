import Image from "next/image";
import Link from "next/link";
import { blogPosts } from "@/lib/siteData";

export function generateStaticParams() {
  return blogPosts.map((post) => ({ slug: post.slug }));
}

export function generateMetadata({ params }) {
  const post = blogPosts.find((entry) => entry.slug === params.slug);
  return {
    title: post?.title || "Blog Post",
    description: post?.excerpt || "KroozTV article",
  };
}

export default async function BlogPostPage({ params }) {
  const { slug } = await params;
  const post = blogPosts.find((entry) => entry.slug === slug);
  if (!post) {
    return <div className="container py-10">Post not found.</div>;
  }

  const related = blogPosts.filter((entry) => entry.slug !== slug).slice(0, 3);

  return (
    <article className="container space-y-8 py-10">
      <header className="space-y-3">
        <h1 className="text-3xl font-bold">{post.title}</h1>
        <p className="text-sm text-zinc-400">{post.date} • {post.author} • 6 min read</p>
      </header>
      <Image src={post.image} alt={post.title} width={1200} height={700} className="rounded-xl object-cover" />
      <div className="space-y-4 text-zinc-200">
        {post.content.map((paragraph) => <p key={paragraph}>{paragraph}</p>)}
      </div>
      <section>
        <h2 className="mb-3 text-2xl font-semibold">Related Posts</h2>
        <div className="grid gap-4 md:grid-cols-3">
          {related.map((item) => (
            <Link key={item.slug} href={`/blog/${item.slug}`} className="rounded-lg border border-zinc-800 bg-zinc-900 p-4 hover:border-zinc-600">
              <h3 className="font-semibold">{item.title}</h3>
              <p className="mt-2 text-sm text-zinc-300">{item.excerpt}</p>
            </Link>
          ))}
        </div>
      </section>
    </article>
  );
}
